#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#define THREADS 1

time_t t_start, t_end, t_exec=0;
int Max_num, Bound;
int *primes;
int total = 0;

int
is_prime (int v)
{
  t_start = time(NULL);

  int i, largest = floor (sqrt (v)) + 1;

  for (i = 0; i < total; i++)   {
    if (primes[i] > largest)
        break;
    if (v % primes[i] == 0)     {
    t_end=time(NULL);
    t_exec+=t_end-t_start;

        return 0;
    }
  }
  t_end=time(NULL);
  t_exec+=t_end-t_start;

  return (v > 1);
}

void *
work (void *arg)
{
  int start;
  int end;
  int i;

  start = (Max_num / THREADS) * ((int) arg);
  if ((start % 2) == 0)
    start++;
  end = start + Max_num / THREADS;
  for (i = start; i < end; i += 2)  {
    if (is_prime (i))   {
      primes[total] = i;
      total++;
    }
  }
  return NULL;
}

int main (int argn, char **argv)
{
  int digit, i, id = 0;
  pthread_t tids[THREADS - 1];

  if (argv[1]==0)
    digit = 10000;
  else
    digit = atoi (argv[1]);
  Max_num = 1<<digit;
  Bound = floor (sqrt (Max_num)) + 1;

  primes = (int *) malloc( Max_num/2);
  
  total = 1;
  primes[0] = 2;

  work ((void *) id);


  printf ("Number of prime numbers between 2 and %d: %d\n", Max_num, total);

  for (i = 0; i < total; i++)  {
      printf ("%d, ", primes[i]);
  }
  printf("\n");
  printf("is_prime time in %ld sec\n",t_exec);

  return 0;
}

