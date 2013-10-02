#include <math.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/time.h>

/***
 * Use parameters:
 * time_t start_time;
 * time_t end_time;
 * time_t exec_time=0;
 ***/
inline void GetTime_time(time_t*);
inline void ComputeTime_time(time_t*,time_t*,time_t*);
inline void ShowTime_time(time_t*);

inline void GetTime_time(time_t* get_time){
    *get_time=time(NULL);
    return;
}
inline void ComputeTime_time(time_t* start_time,time_t* end_time,time_t* exec_time){
    *exec_time+=*end_time-*start_time;
    return;
}
inline void ShowTime_time(time_t* exec_time){
    printf("Program runs in %lld secs\n",(long long int)*exec_time);
    return;
}

/***
 * Use parameters:
 * struct timespec start_time;
 * struct timespec end_time;
 * long long int exec_time_sec=0;
 * long long int exec_time_nsec=0;
 ***/
inline void GetTime_clock(struct timespec*);
inline void ComputeTime_clock(struct timespec*,struct timespec*,long long int*,long long int*);
inline void ShowTime_clock(long long int*,long long int*);

inline void GetTime_clock(struct timespec* get_time){
    clock_gettime(CLOCK_REALTIME, get_time);
    return;
}
inline void ComputeTime_clock(struct timespec* start_time,struct timespec* end_time,long long int* exec_time_sec,long long int* exec_time_nsec){
    if(end_time->tv_nsec > start_time->tv_nsec){
        *exec_time_sec+=(long long int)end_time->tv_sec - (long long int)start_time->tv_sec;
        *exec_time_nsec+=(long long int)end_time->tv_nsec - (long long int)start_time->tv_nsec;
    }
    else{
        *exec_time_sec+=(long long int)end_time->tv_sec - (long long int)start_time->tv_sec - 1L;
        *exec_time_nsec+=(long long int)end_time->tv_nsec - (long long int)start_time->tv_nsec + 1000000000;
    }
    if(*exec_time_nsec/1000000000 >0){
        *exec_time_nsec-=1000000000;
        *exec_time_sec+=1;
    }
    return;
}
inline void ShowTime_clock(long long int* exec_time_sec, long long int* exec_time_nsec){
    printf("Program runs in %lld.%09lld secs\n",*exec_time_sec,*exec_time_nsec);
    return;
}

/***
 * Use parameters:
 * struct rusage start_time;
 * struct rusage end_time;
 * long long int exec_time_sec=0;
 * long long int exec_time_usec=0;
 ***/
inline void GetTime_rusage(struct rusage*);
inline void ComputeTime_rusage(struct rusage*,struct rusage*,long long int*,long long int*);
inline void ShowTime_rusage(long long int*,long long int*);

inline void GetTime_rusage(struct rusage* get_time){
    getrusage(RUSAGE_SELF, get_time);
    return;
}
inline void ComputeTime_rusage(struct rusage* start_time,struct rusage* end_time,long long int* exec_time_sec,long long int* exec_time_usec){
    if(end_time->ru_utime.tv_usec > start_time->ru_utime.tv_usec){
        *exec_time_sec+=(long long int)end_time->ru_utime.tv_sec - (long long int)start_time->ru_utime.tv_sec;
        *exec_time_usec+=(long long int)end_time->ru_utime.tv_usec - (long long int)start_time->ru_utime.tv_usec;
    }
    else{
        *exec_time_sec+=(long long int)end_time->ru_utime.tv_sec - (long long int)start_time->ru_utime.tv_sec - 1L;
        *exec_time_usec+=(long long int)end_time->ru_utime.tv_usec - (long long int)start_time->ru_utime.tv_usec + 1000000;
    }
    if(*exec_time_usec/1000000 >0){
        *exec_time_usec-=1000000;
        *exec_time_sec+=1;
    }
    return;
}
inline void ShowTime_rusage(long long int* exec_time_sec, long long int* exec_time_usec){
    printf("Program runs in %lld.%06lld secs\n",*exec_time_sec,*exec_time_usec);
    return;
}

/***
 * Use parameters:
 * unsigned long long start_clock;
 * unsigned long long end_clock;
 * unsigned long long exec_clock=0;
 ***/
#define rdtsc(low,high) __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))
#define rdtscll(val) __asm__ __volatile__("rdtsc" : "=A" (val))
inline void GetTime_rdtsc(unsigned long long*);
inline void ComputeTime_rdtsc(unsigned long long*,unsigned long long*,unsigned long long*);
inline void ShowTime_rdtsc(unsigned long long*);

inline void GetTime_rdtsc(unsigned long long* get_clock){
    rdtscll(*get_clock);
    return;
}
inline void ComputeTime_rdtsc(unsigned long long* start_clock,unsigned long long* end_clock,unsigned long long* exec_clock){
    *exec_clock+=*end_clock-*start_clock;
    return;
}
inline void ShowTime_rdtsc(unsigned long long* exec_clock){
    printf("Program runs in %lld CPU clocks\n",*exec_clock);
    return;
}

