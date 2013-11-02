#include <stdlib.h>
#include <stdio.h>
#include <time.h>
main ()
{
  int count, num= 1073741824;
  double pi = 0;
  time_t startwtime, endwtime;

  //compute time setting
  startwtime = time (NULL);

  for (count = 1; count <= num; count++)
    {
      if (count % 2)
	pi = pi + (1.0 / (2.0 * count - 1));
      else
	pi = pi - (1.0 / (2.0 * count - 1));
    }

  pi = pi * 4;

  printf("The approximate value of pi = %f\n",pi);

  //compute total time
  printf ("wall clock time = %d\n",(int)(endwtime - startwtime));

  return 0;
}
