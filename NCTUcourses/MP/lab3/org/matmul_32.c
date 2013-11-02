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
		for (j = 0; j < dim3; j++)
			C[i][j] = 0.;
		for (k = 0; k < dim2; k++) {
            int max=dim3-32;
			for (j = 0; j < max; j+=32){
				C[i][j] += A[i][k]*B[k][j];
				C[i][j+1] += A[i][k]*B[k][j+1];
				C[i][j+2] += A[i][k]*B[k][j+2];
 				C[i][j+3] += A[i][k]*B[k][j+3];
 				C[i][j+4] += A[i][k]*B[k][j+4];
 				C[i][j+5] += A[i][k]*B[k][j+5];
 				C[i][j+6] += A[i][k]*B[k][j+6];
 				C[i][j+7] += A[i][k]*B[k][j+7];
 				C[i][j+8] += A[i][k]*B[k][j+8];
 				C[i][j+9] += A[i][k]*B[k][j+9];
 				C[i][j+10] += A[i][k]*B[k][j+10];
				C[i][j+11] += A[i][k]*B[k][j+11];
				C[i][j+12] += A[i][k]*B[k][j+12];
 				C[i][j+13] += A[i][k]*B[k][j+13];
 				C[i][j+14] += A[i][k]*B[k][j+14];
 				C[i][j+15] += A[i][k]*B[k][j+15];
 				C[i][j+16] += A[i][k]*B[k][j+16];
 				C[i][j+17] += A[i][k]*B[k][j+17];
 				C[i][j+18] += A[i][k]*B[k][j+18];
 				C[i][j+19] += A[i][k]*B[k][j+19];
 				C[i][j+20] += A[i][k]*B[k][j+20];
				C[i][j+21] += A[i][k]*B[k][j+21];
				C[i][j+22] += A[i][k]*B[k][j+22];
 				C[i][j+23] += A[i][k]*B[k][j+23];
 				C[i][j+24] += A[i][k]*B[k][j+24];
 				C[i][j+25] += A[i][k]*B[k][j+25];
 				C[i][j+26] += A[i][k]*B[k][j+26];
 				C[i][j+27] += A[i][k]*B[k][j+27];
 				C[i][j+28] += A[i][k]*B[k][j+28];
 				C[i][j+29] += A[i][k]*B[k][j+29];
 				C[i][j+30] += A[i][k]*B[k][j+30];
				C[i][j+31] += A[i][k]*B[k][j+31];
            }
			C[i][992] += A[i][k]*B[k][992];
			C[i][993] += A[i][k]*B[k][993];
			C[i][994] += A[i][k]*B[k][994];
 			C[i][995] += A[i][k]*B[k][995];
 			C[i][996] += A[i][k]*B[k][996];
 			C[i][997] += A[i][k]*B[k][997];
 			C[i][998] += A[i][k]*B[k][998];
 			C[i][999] += A[i][k]*B[k][999];
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

