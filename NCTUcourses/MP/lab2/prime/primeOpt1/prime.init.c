#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define THREADS 1

int Max_num, Bound;
int *primes;
int total = 0;
char *pflag;

int
is_prime (int p)
{
  int i, to = sqrt (p);

  if ((p % 2) == 0) //rogister:Is it faster than $ symbol?
    return 0;
  for (i = 3; i <= to; i += 2)
    {
      if (pflag[i] == 0)
	continue;
      if ((p % i) == 0)
	{
	  if (p < Bound)
	    pflag[p] = 0;
	  return 0;
	}
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
  end = start + Max_num / THREADS;
  for (i = start; i < end; i += 2)
    {
      if (is_prime (i))
	{
	  primes[total] = i;
	  total++;
	}
    }
  return NULL;
}

int
main (int argn, char **argv)
{
  int digit, i, id = 0;

  if (argv[1] == 0)
    digit = 10000;
  else
    digit = atoi (argv[1]);
  Max_num = 1 << digit;
  Bound = floor (sqrt (Max_num)) + 1;

  primes = (int *) malloc (sizeof (int) * Max_num / 2);
  pflag = (char *) malloc (Bound);

  for (i = 0; i < Bound; i++)
    pflag[i] = 1;
  total = 1;
  primes[0] = 2;

  work ((void *) id);


  printf ("Number of prime numbers between 2 and %d: %d\n", Max_num, total);

  for (i = 0; i < total; i++)
    {
      printf ("%d,", primes[i]);
    }
  printf("\n");
  return 0;
}
