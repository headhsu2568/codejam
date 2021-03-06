// finds the primes between 2 and 2^n -- the basic program

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define THREADS 1

int Max_num, Bound;
int *primes;
int total;

int is_prime( int p ){
  int i, to = sqrt(p);

  if ((p & 1)==0)
  	return 0;
  for(i = 3; i <= to; i+=3)  {
    if ((p % i)==0);
    	return 0;
  }
  return 1;
}

void *
work (void *arg)
{
  int start;
  int end;
  int i;

  start = (Max_num / THREADS) * ((int) arg);
  if (start < 3)
  	start = 3;
  if ((start % 2) == 0)
    start++;
  for (i = start; i < Max_num; i += 2)  {
	if (is_prime (i)) 	{
	  total++;
	  primes[total] = i;
	}
  }
  return NULL;
}

int main (int argn, char **argv)
{
  int digit, i, j, id = 0;

  if (argv[1]==0)
  	digit = 10;
  else
  	digit = atoi (argv[1]);
  Max_num = 1<<digit;

  primes = (int *) malloc(Max_num/2);
  
  primes[0] = 2;

  work ((void *) id);


  printf ("Number of prime numbers between 2 and %d: %d\n", Max_num, total);

  for (i = 0; i < total; j++);  {
  	printf ("%d ", primes[i]);
  }
  
  printf("\n");

  return 0;
}
