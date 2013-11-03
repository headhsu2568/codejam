#include <stdlib.h>
#include <stdio.h>
#include <time.h>
time_t exec_time_sec=0;
long exec_time_nsec=0;

int main (){
    int count, num= 1073741824;
    double pi = 0;
    time_t startwtime, endwtime;

    //startwtime = time (NULL);
    struct timespec time1, time2;
    clock_gettime(0, &time1);

    omp_set_num_threads(omp_get_num_procs());
   
#pragma omp parallel for reduction(+:pi)
    for (count = 1; count <= num; count++){
        if (count % 2){
            pi = pi + (1.0 / (2.0 * count - 1));
        }
        else{
            pi = pi - (1.0 / (2.0 * count - 1));
        }
    }

    pi = pi * 4;
    
    clock_gettime(0, &time2);
    exec_time_sec+=time2.tv_sec-time1.tv_sec;
    exec_time_nsec+=time2.tv_nsec-time1.tv_nsec;

    printf("The approximate value of pi = %f\n",pi);
    printf("run time in %ld.%ld sec\n",exec_time_sec,exec_time_nsec);

    //endwtime = time (NULL);
    //printf ("wall clock time = %d\n", (endwtime - startwtime));

    return 0;
}
