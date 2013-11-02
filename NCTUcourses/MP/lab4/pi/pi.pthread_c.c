#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/time.h>

#define MAX_THREAD 1024

typedef struct{
    int count;
    int num;
    double pi;
}pi_arg;

void *pi_compute(void *arg){
    pi_arg *thread_arg= (pi_arg *) arg;
    int count=thread_arg->count, num=thread_arg->num;
    double pi=0.0;

    for(; count <= num; ++count){
        if (count & 1)
            pi = pi + (1.0 / (double)((count<<1) - 1));
        else
            pi = pi - (1.0 / (double)((count<<1) - 1));
    }
    thread_arg->pi=pi;
    return;
}

int main (int argc,char *argv[]){
    int n, count, num=1073741824, part_num, i;
    double pi = 0.0;
    time_t startwtime, endwtime;
    pthread_t *threads;
    pthread_attr_t pthread_custom_attr;
    pi_arg *threads_arg;

    // check for input parameter n thread
    if (argc != 2){
        printf ("Usage: %s n\n  where n is no. of thread\n", argv[0]);
        exit (1);
    }
    n = atoi (argv[1]);
    if ((n < 1) || (n > MAX_THREAD)){
        printf ("The no of thread should between 1 and %d.\n", MAX_THREAD);
        exit (1);
    }

    // initial n threads
    threads = (pthread_t *) malloc (n * sizeof (*threads));
    pthread_attr_init (&pthread_custom_attr);
    threads_arg = (pi_arg *) malloc (n * sizeof (*threads_arg));
    part_num=num/n;
    count=1;

    //compute time setting
    long int exec_time_usec=0;
    struct rusage time1,time2;
    int ret;
    ret = getrusage(RUSAGE_SELF, &time1);
    startwtime = time (NULL);

    // threads creating
    for(i=0;i<n;++i){
        threads_arg[i].count=count;
        count=count+part_num;
        threads_arg[i].num=((count-1)<num)?(count-1):num;
        pthread_create(&threads[i], &pthread_custom_attr, pi_compute, (void *)(threads_arg +i));
    }

    // finally compute pi
    for(i=0;i<n;++i){
        pthread_join(threads[i],NULL);
        pi=pi + threads_arg[i].pi;
    }
    pi = pi*4 ;

    printf("The approximate value of pi = %f\n",pi);
    
    //compute total time
    ret = getrusage(RUSAGE_SELF, &time2);
    exec_time_usec+=(time2.ru_utime.tv_usec+time2.ru_stime.tv_usec-time1.ru_utime.tv_usec-time1.ru_stime.tv_usec);
    endwtime = time (NULL);
    printf ("wall clock time = %d\n",(int)(endwtime - startwtime));
    printf ("run in %f secs (%ld usecs)\n",(double)(exec_time_usec/1000000.0),exec_time_usec);

    return 0;
}
