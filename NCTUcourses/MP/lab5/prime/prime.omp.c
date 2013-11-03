// finds the primes between 2 and 2^n -- the basic program

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define THREADS 1

unsigned int Max_num, Bound;
int *primes;
int total = 0;
time_t exec_time_sec=0;
double exec_time_nsec=0.0;
double exec_time=0.0;

int is_prime( int p ){
    int i, to = sqrt(p);

    if ((p & 1)==0)
        return 0;
    for(i = 3; i <= to; i+=2){
        if ((p % i)==0)
            return 0;
    }
    return 1;
}

void *work (void *arg)
{
    int start;
    int end;
    int i;

    start = (Max_num / THREADS) * ((int) arg);
    if (start < 3)
        start = 3;
    if ((start % 2) == 0)
        start++;
    #pragma omp parallel for
    for (i = start; i < Max_num; i += 2){
        if (is_prime (i)){
            #pragma omp critical
            primes[total++] = i;
        }
    }
    return NULL;
}

int main (int argn, char **argv)
{
    int digit, i, id = 0;

    if (argv[1] == 0)
        Max_num = 1 << 24;
    else
        Max_num = 1 << atoi (argv[1]);

    struct timespec time1, time2;
    clock_gettime(0, &time1);

    primes = (int *) malloc(Max_num/2);

    total = 1;
    primes[0] = 2;

    work ((void *) id);

    clock_gettime(0, &time2);
    exec_time_sec+=time2.tv_sec-time1.tv_sec;
    exec_time_nsec+=(double)time2.tv_nsec-(double)time1.tv_nsec;
    exec_time_nsec=exec_time_nsec/1000000000.0;
    exec_time=(double)exec_time_sec+exec_time_nsec;

    printf ("Number of prime numbers between 2 and %d: %d\n", Max_num, total);

//    for (i = 0; i < total; i++){
//        printf ("%d ", primes[i]);
//    }

    printf("\n");
    printf("run in %lf secs\n",exec_time);

}
