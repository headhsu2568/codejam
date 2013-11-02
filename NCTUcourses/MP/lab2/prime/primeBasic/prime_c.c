// finds the primes between 2 and 2^n -- the basic program

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <sys/resource.h>

#define THREADS 1

int Max_num, Bound;
int *primes;
int total = 0;
long int exec_time_sec=0;
long int exec_time_usec=0;

int
is_prime (int p)
{
  struct rusage time1,time2;
  int ret;
  ret = getrusage(RUSAGE_SELF, &time1);
  int i, to = sqrt (p);

  if ((p & 1) == 0)
    return 0;
  for (i = 3; i <= to; i += 2)
    {
      if ((p % i) == 0){
  ret = getrusage(RUSAGE_SELF, &time2);
  exec_time_sec+=(time2.ru_utime.tv_sec+time2.ru_stime.tv_sec-time1.ru_utime.tv_sec-time1.ru_stime.tv_sec);
  exec_time_usec+=(time2.ru_utime.tv_usec+time2.ru_stime.tv_usec-time1.ru_utime.tv_usec-time1.ru_stime.tv_usec);
	return 0;
      }
    }
  ret = getrusage(RUSAGE_SELF, &time2);
  exec_time_sec+=(time2.ru_utime.tv_sec+time2.ru_stime.tv_sec-time1.ru_utime.tv_sec-time1.ru_stime.tv_sec);
  exec_time_usec+=(time2.ru_utime.tv_usec+time2.ru_stime.tv_usec-time1.ru_utime.tv_usec-time1.ru_stime.tv_usec);
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

  for (i = start; i < Max_num; i += 2)
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

  primes = (int *) malloc (Max_num / 2);

  total = 1;
  primes[0] = 2;

  work ((void *) id);


  printf ("Number of prime numbers between 2 and %d: %d\n", Max_num, total);

  for (i = 0; i < total; i++)
    {
      printf ("%d,", primes[i]);
    }
  printf("\n");

  printf("is_prime time in %ld sec\n",exec_time_sec);
  printf("is_prime time in %ld usec\n",exec_time_usec);
  return 0;
}
