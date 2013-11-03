#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

int NUM_LOOPS = 32;
int NUM_THREADS = 2;

#define DIM (1000)
#define PRIME 1046527  

#define true 1
#define false 0
#define CONSUMER_TURN 1
#define PRODUCER_TURN 0
#define bool int

struct timespec start_time;
struct timespec end_time;


#define _DEFINE_LEVEL1_DCACHE_LINESIZE 64
#define __shared_write __attribute__((aligned (_DEFINE_LEVEL1_DCACHE_LINESIZE)))  

#ifndef __cacheline_aligned
#define __cacheline_aligned __attribute__((__aligned__(_DEFINE_LEVEL1_DCACHE_LINESIZE), __section__(".data.cacheline_aligned")))
#endif /* __cacheline_aligned */
#define __read_mostly __attribute__((__section__(".data.read_mostly")))

// circular fifo
volatile __cacheline_aligned struct fifo
{
    volatile unsigned int array[DIM];
    volatile __shared_write unsigned int head_index;
    volatile __shared_write unsigned int tail_index;
    volatile __shared_write bool is_full;
    volatile __shared_write bool is_empty;  
    volatile __shared_write int turn;
};
typedef volatile struct fifo fifo_t;

struct barrier
{
    int counter;
    pthread_mutex_t lock;
    pthread_cond_t producer_cond;
    pthread_cond_t consumer_cond;
};
typedef struct barrier barrier_t;

barrier_t* bar_g = NULL;
volatile void* consumer(fifo_t* fifo_array)
{    
    assert(fifo_array);

    int count = 0; 
    int i;
    int j;

    unsigned int sig = 0; 
    barrier_t* bar=bar_g;

    while(1)
    { 
        pthread_mutex_lock(&(bar->lock));
        pthread_cond_wait(&(bar->consumer_cond),&(bar->lock));
        pthread_mutex_unlock(&(bar->lock));
        //printf("consumer\n"); fflush(stdout);

        for(i = 0; i < NUM_THREADS; i++)
        {      
            fifo_t* fifo = fifo_array + i;

            for(j = 0; j < DIM; j++)
            {         
                assert(fifo->is_empty == false);
                assert(fifo->tail_index < DIM);

                sig ^= fifo->array[fifo->tail_index];
                sig += (sig >> 16);

                fifo->tail_index = (fifo->tail_index+1) % DIM;
                fifo->is_full = false;
                fifo->is_empty = (fifo->head_index == fifo->tail_index) ? true : false;
            }                      
        }

        barrier_func(bar);       
        count++;
        if(count == NUM_LOOPS){
            break;
        }
    }
    printf("sig=%x\n",sig); 

    return;
}  

volatile void* producer(fifo_t* fifo)
{ 
    assert(fifo);

    int i;
    int count = 0;
    int seed = 0x12345678;
    barrier_t* bar=bar_g;

    while(1)
    {   
            //printf("producer\n"); fflush(stdout);
            for(i = 0; i < DIM; i++)
            {                  
                seed ^= seed + (seed >> 13) + (count - 543) + count + i;
                assert(fifo->is_full == false);
                assert(fifo->head_index < DIM);        
                fifo->array[fifo->head_index] = seed; // generate a random number       
                fifo->head_index = (fifo->head_index+1) % DIM;
                fifo->is_full = fifo->head_index == fifo->tail_index ? true : false;
                fifo->is_empty = false;   
            }    

            barrier_func(bar);
            count++;
            if(count == NUM_LOOPS)        
                break;
    }     
    return;
} 

void barrier_init(barrier_t* bar)
{
    bar->counter=0;
    pthread_mutex_init(&(bar->lock),NULL);
    pthread_cond_init(&(bar->producer_cond),NULL);
    pthread_cond_init(&(bar->consumer_cond),NULL);
    return;
}

void barrier_func(barrier_t* bar)
{
    pthread_mutex_lock(&(bar->lock));
    if(bar->counter==NUM_THREADS){
        bar->counter=0;
        pthread_cond_broadcast(&(bar->producer_cond));
        pthread_mutex_unlock(&(bar->lock));
    }
    else{
        bar->counter++;
        if(bar->counter==NUM_THREADS){
            pthread_cond_signal(&(bar->consumer_cond));
        }
        while(pthread_cond_wait(&(bar->producer_cond), &(bar->lock))!=0){}
        pthread_mutex_unlock(&(bar->lock));
    }
    return;
}

int main (int argc, char *argv[])
{  
    int i;
    int j;
    unsigned int seed = 0x1234567;

    assert(argc == 3);    
    NUM_THREADS = atoi(argv[1]);
    NUM_LOOPS = atoi(argv[2]);

    printf("NUM_THREADS:%d, NUM_LOOPS:%d\n", NUM_THREADS, NUM_LOOPS);

    pthread_t* thread_array = NULL;
    thread_array = malloc(sizeof(thread_array[0])*NUM_THREADS);

    fifo_t* fifo_array = NULL;
    fifo_array = malloc(sizeof(fifo_array[0])*NUM_THREADS);
    bar_g = malloc(sizeof(struct barrier));
    barrier_init(bar_g);

    for(i = 0; i < NUM_THREADS; i++)
    {    
        fifo_t* fifo = fifo_array + i;
        for(j = 0; j < DIM; j++)
        {
            seed = seed ^ (seed > 15) + i;
            fifo->array[j] = seed;    
        }
        //memset(fifo->array, 0, sizeof(fifo->array[0])*DIM);
        fifo->head_index = 0;
        fifo->tail_index = 0;
        fifo->is_full = false;
        fifo->is_empty = true;
    }

    pthread_attr_t attr;
    int rc;  
    void *status;   


    /* Initialize and set thread detached attribute */  
    pthread_attr_init(&attr);  
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);   

    clock_gettime(CLOCK_REALTIME, &start_time);                

    for(i = 0; i < NUM_THREADS; i++)  
        rc = pthread_create(&(thread_array[i]), &attr, (void *)producer, (void *)(fifo_array + i));             

    consumer(fifo_array);

    for(i = 0; i < NUM_THREADS; i++)  
        pthread_join(&(thread_array[i]), &status);    

    clock_gettime(CLOCK_REALTIME, &end_time);                

    pthread_attr_destroy(&attr);

    printf("Main: program completed. Exiting.\n");  
    printf("s_time.tv_sec:%lld, s_time.tv_nsec:%09lld\n", (long long int)start_time.tv_sec, (long long int)start_time.tv_nsec);
    printf("e_time.tv_sec:%lld, e_time.tv_nsec:%09lld\n", (long long int)end_time.tv_sec, (long long int)end_time.tv_nsec);
    if(end_time.tv_sec == start_time.tv_sec)
    {
        printf("diff_time:%lld.%09lld\n", 
                (long long int)(end_time.tv_sec - start_time.tv_sec), 
                (long long int)(end_time.tv_nsec - start_time.tv_nsec));
    }
    else
    {
        printf("diff_time:%lld.%09lld\n", 
                (long long int)(end_time.tv_sec - start_time.tv_sec - 1), 
                (long long int)(1000000000 + end_time.tv_nsec - start_time.tv_nsec));
    }
    return 0;
}
