#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <errno.h>
#include <unistd.h>
#include "headtime.h"

// Variables
int* h_A = NULL;
int* h_B = NULL;
int* h_C = NULL;
int* d_A = NULL;
int* d_B = NULL;
int* d_C = NULL;

int N = 10;  
unsigned int seed = 0x1234567;

timespec start_time;                                 
timespec end_time;
struct timespec start_time2;
struct timespec end_time2;
long long int exec_time_sec=0;
long long int exec_time_nsec=0;

void array_mul(int* A, int* B, int* C, int N) 
{        
    for(int i = 0; i < N; i++) 
    {
        for(int j = 0; j < N; j++) 
        {          
            for(int k = 0; k < N; k++) 
            {
                C[i*N+j] = C[i*N+j] + A[i*N+k] * B[k*N+j]; 
            }  
        }      
    }
};

unsigned int myrand(unsigned int *seed, unsigned int input)
{  
    *seed = (*seed << 13) ^ (*seed >> 15) + input + 0xa174de3;
    return *seed;
};

void sig_check()
{    
    unsigned int sig = 0x1234567;
    for(int i = 0; i < N; i++)
    {    
        myrand(&sig, h_C[i]);    
    }           

    printf("Computed check sum signature:0x%08x\n", sig);
    if(sig == 0x9f3afc72)
        printf("Result check by signature successful!!\n");
    else
        printf("Result check by signature failed!!\n");
}

void show_array(int* array)
{
    for(int i = 0; i < N; i++)
    {
        for(int j = 0; j < N; j++)
            printf("%13d, ", array[i*N+j]);
        printf("\n");  
    }
    printf("\n");
}

int main (int argc, char *argv[])
{ 
    // get the dimension of array    
    assert(argc == 2);  
    N = atoi(argv[1]);  
    int size = N*N*sizeof(int);

    printf("N:%d, size:%d\n", N, size);

    // Allocate input vectors h_A and h_B in host memory
    h_A = (int*)malloc(size);  
    h_B = (int*)malloc(size);  
    h_C = (int*)malloc(size);
    assert(h_A);  
    assert(h_B);  
    assert(h_C);  

    // initial array A & B
    for(int i = 0; i < N; i++)
        for(int j = 0; j < N; j++)
        {   
            h_A[i*N+j] = myrand(&seed, i*j) & 0xff;
            h_B[i*N+j] = myrand(&seed, i*j) & 0xff;
            h_C[i*N+j] = 0; 
        }

    clock_gettime(CLOCK_REALTIME, &start_time);    

    GetTime_clock(&start_time2);
    array_mul(h_A, h_B, h_C, N);
    GetTime_clock(&end_time2);

    clock_gettime(CLOCK_REALTIME, &end_time);    

    ComputeTime_clock(&start_time2,&end_time2,&exec_time_sec,&exec_time_nsec);
    ShowTime_clock(&exec_time_sec,&exec_time_nsec);

    printf("sizeof(start_time.tv_sec):%d, sizeof(start_time.tv_nsec):%d\n", sizeof(start_time.tv_sec), sizeof(start_time.tv_nsec));
    printf("s_time.tv_sec:%d, s_time.tv_nsec:%d\n", start_time.tv_sec, start_time.tv_nsec);
    printf("e_time.tv_sec:%d, e_time.tv_nsec:%d\n", end_time.tv_sec, end_time.tv_nsec);
    double execution_time = (double)end_time.tv_sec + (double)end_time.tv_nsec/1000000000.0 
        - (double)start_time.tv_sec - (double)start_time.tv_nsec/1000000000.0;
    printf("diff_time:%.4f(s)\n", execution_time);

    //show_array(h_A);
    //show_array(h_B);
    //show_array(h_C);

    //sig_check();         

    return 0;
}



