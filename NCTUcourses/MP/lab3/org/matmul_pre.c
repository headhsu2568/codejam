#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PREFETCHT0(x) __asm__ __volatile__ ("PREFETCHT0 %0": : "m"(*(char *)(x)))
#define PREFETCHT1(x) __asm__ __volatile__ ("PREFETCHT1 %0": : "m"(*(char *)(x)))
#define PREFETCHT2(x) __asm__ __volatile__ ("PREFETCHT2 %0": : "m"(*(char *)(x)))  
#define PREFETCHNTA(x) __asm__ __volatile__ ("PREFETCHNTA %0": : "m"(*(char *)(x)))

#define dim1 1000
#define dim2 1000
#define dim3 1000
#define PRIME 1046527  

unsigned A[dim1][dim2], B[dim2][dim3], C[dim1][dim3];

work()
{
	int i, j, k;
	double start, finish;

	start = clock();
	for (i = 0; i < dim1; i++)
	{
        PREFETCHNTA(A[i]);
		for (j = 0; j < dim3; j++){
			C[i][j] = 0.;
        }
        PREFETCHNTA(C[i]);
		for (k = 0; k < dim2; k++) {
            PREFETCHNTA(B[k]);
			for (j = 0; j < dim3; j++){
				C[i][j] += A[i][k]*B[k][j];
            }
		}
	}
	finish = clock();
	
	printf("Computing time for C(%d,%d) = A(%d,%d) X B(%d,%d) is %.0f s\n", 
		   dim1, dim3, dim1, dim2, dim2, dim3, (finish - start)/CLOCKS_PER_SEC);	
}

void sig_check()   // result check by signature
{
  unsigned sig = 0;
	int i,j;
	
	for(i = 0; i < dim1; i++)   
		for (j=0; j<dim3; j++) {
				sig ^= C[i][j];
				sig += (sig >> 16);
		}
  
  printf("Computed check sum signature:0x%08x\n", sig);
  if(sig == 0x869f1aa5)
    printf("Result check by signature successful!!\n");
  else
    printf("Result check by signature failed!!\n");
}


int main()
{
	int i, j, k;
	unsigned seed=0;

	seed = PRIME;    // initialized by any prime
	for (i = 0; i < dim1; i++)
		for (j = 0; j < dim2; j++)
			A[i][j] = seed++*PRIME;
	for (i = 0; i < dim2; i++)
		for (j = 0; j < dim3; j++)
			B[i][j] = seed++*PRIME;
		
	work();
	sig_check();

	return 0;
}

