#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <errno.h>
#include <unistd.h>
#include <cutil_inline.h>

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

__global__ void MatrixMulKernel(int* d_A, int* d_B, int* d_C, int width){
    int Cvalue=0;
    for(int k=0; k<width; ++k){
        int Aelement = d_A[blockIdx.x*width+k];
        int Belement = d_B[k*width+threadIdx.y];
        Cvalue += Aelement * Belement;
    }
    d_C[blockIdx.x*width+threadIdx.y] = Cvalue;
}

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


    // Allocate and copy
    cudaMalloc(&d_A, size);
    cudaMalloc(&d_B, size);
    cudaMalloc(&d_C, size);
    cudaMemcpy(d_A, h_A, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B, size, cudaMemcpyHostToDevice);

    clock_gettime(CLOCK_REALTIME, &start_time);

    // Invoke kernel
    dim3 dimGrid(1, 1);
    dim3 dimBlock(N, N);

    // Launch the device computation
    MatrixMulKernel<<<dimGrid, dimBlock>>>(d_A, d_B, d_C, N);

    cudaMemcpy(h_C, d_C, size, cudaMemcpyDeviceToHost);

    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

    //array_mul(h_A, h_B, h_C, N);    

    clock_gettime(CLOCK_REALTIME, &end_time);    

    printf("sizeof(start_time.tv_sec):%d, sizeof(start_time.tv_nsec):%d\n", sizeof(start_time.tv_sec), sizeof(start_time.tv_nsec));
    printf("s_time.tv_sec:%d, s_time.tv_nsec:%d\n", start_time.tv_sec, start_time.tv_nsec);
    printf("e_time.tv_sec:%d, e_time.tv_nsec:%d\n", end_time.tv_sec, end_time.tv_nsec);
    double execution_time = (double)end_time.tv_sec + (double)end_time.tv_nsec/1000000000.0 
        - (double)start_time.tv_sec - (double)start_time.tv_nsec/1000000000.0;
    printf("diff_time:%.4f(s)\n", execution_time);

    //show_array(h_A);
    //show_array(h_B);
    //show_array(h_C);

    sig_check();         

    return 0;
}
