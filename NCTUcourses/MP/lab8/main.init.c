#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

typedef struct stock stock_t;
struct stock
{
    unsigned int company_a;
    unsigned int company_b;
    unsigned int company_c;
    unsigned int index;
    unsigned int seed;
};

int day_num = 0;
int stock_num = 0;
int thread_num = 0;

pthread_t* thread_array = NULL;
stock_t* stock_array = NULL;

pthread_barrier_t bar;

void delay()
{
    int i;
    int count = rand() & 0xff;
    for(i = 0; i < count; i++)
        pthread_yield();
}

unsigned int myrand(unsigned int *seed, unsigned int input)
{
    *seed = (*seed << 13) ^ (*seed >> 15) + input + 0xa174de3;
    return *seed;
}

void sig_check()
{
    int i;
    int sig = 0x1234567;
    for(i = 0; i < stock_num; i++)
    {
        myrand(&sig, stock_array[i].company_a);
        myrand(&sig, stock_array[i].company_b);
        myrand(&sig, stock_array[i].company_c);
        myrand(&sig, stock_array[i].index);
        myrand(&sig, stock_array[i].seed);
    }

    printf("Computed check sum signature:0x%08x\n", sig);
    if(sig == 0x869f1aa5)
        printf("Result check by signature successful!!\n");
    else
        printf("Result check by signature failed!!\n");
}

int stock_sim(int* index, int* company)
{
    // This function randomly generates a varivation for a stock's index.
    int inc = 0;
    static int seed = 123;

    inc = myrand(&seed, *index) + *company;
    inc = (inc >> 15) ^ (inc << 13);
    inc = (inc >> 17) ^ (inc << 11);
    inc = inc >> 24;
    inc = (*company + inc > 4) ? inc : 5;

    return inc;
};

void* analyzer(void* arg)
{
    int i, j;
    int threadid = *((int*)(&arg));

    printf("threadid:%d\n", threadid); fflush(stdout);
    pthread_barrier_wait(&bar);

    for(i = 0; i < day_num; i++)
    {
        for(j = threadid; j < stock_num; j = j + thread_num)
        {
            stock_t* stock = stock_array + j;
            stock->company_a += stock_sim(&stock->index, &stock->company_a);
            stock->company_b += stock_sim(&stock->index, &stock->company_b);
            stock->company_c += stock_sim(&stock->index, &stock->company_c);
            stock->index = stock->company_a + stock->company_b + stock->company_c;
        }
    }
}

void show_stock_array()
{
    int i;
    for(i = 0; i < stock_num; i++)
        printf("a:%8d, b:%8d, c:%8d, i:%8d, s:%08x\n",
                stock_array[i].company_a,
                stock_array[i].company_b,
                stock_array[i].company_c,
                stock_array[i].index,
                stock_array[i].seed);
}

int main (int argc, char *argv[])
{
    int i, rc;
    pthread_attr_t attr;

    assert(argc == 4);
    thread_num = atoi(argv[1]);
    stock_num = atoi(argv[2]);
    day_num = atoi(argv[3]);

    // Get L1 cache line size here.
    printf("_SC_LEVEL1_DCACHE_LINESIZE:%ld\n", sysconf(_SC_LEVEL1_DCACHE_LINESIZE));
    printf("sizeof(stock_t):%d\n", (int)sizeof(stock_t));
    printf("thread_num:%d\n", thread_num);
    printf("stock_num:%d\n", stock_num);
    printf("day_num:%d\n", day_num);

    pthread_barrier_init(&bar, NULL, thread_num);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // prepare stock_array
    stock_array = malloc(sizeof(stock_array[0])*(stock_num+1));

    printf("stock_array:%p\n", stock_array);
    assert(stock_array);
    for(i = 0; i < stock_num; i++)
    {
        unsigned int seed = i;

        stock_array[i].company_a = myrand(&seed, 0) & 0xffff;
        stock_array[i].company_b = myrand(&seed, 0) & 0xffff;
        stock_array[i].company_c = myrand(&seed, 0) & 0xffff;
        stock_array[i].index = myrand(&seed, 0) & 0x3ffff;
        stock_array[i].seed = myrand(&seed, 0);
    }

    //initial thread creation
    thread_array = (pthread_t*) malloc(sizeof(pthread_t) * thread_num);
    assert(thread_array);
    for(i = 0; i < thread_num; i++)
    {
        int rc = pthread_create(thread_array+i, &attr, analyzer, (void *)i);
        assert(rc == 0);
    }

    for(i = 0; i < thread_num; i++)
        pthread_join(thread_array[i], NULL);

    //sig_check();

    //show_stock_array();

    return 0;
}
