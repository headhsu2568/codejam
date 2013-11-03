#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

int NUM_LOOPS = 32;
int NUM_THREADS = 2;

#define DIM (1000)
#define PRIME 1046527  

#define true 1
#define false 0
#define CONSUMER_TURN 1
#define PRODUCER_TURN 0
#define bool int

struct timespec start_time;
struct timespec end_time;


#define _DEFINE_LEVEL1_DCACHE_LINESIZE 64
#define __shared_write __attribute__((aligned (_DEFINE_LEVEL1_DCACHE_LINESIZE)))  

#ifndef __cacheline_aligned
#define __cacheline_aligned __attribute__((__aligned__(_DEFINE_LEVEL1_DCACHE_LINESIZE), __section__(".data.cacheline_aligned")))
#endif /* __cacheline_aligned */
#define __read_mostly __attribute__((__section__(".data.read_mostly")))


// circular fifo
volatile __cacheline_aligned struct fifo
{
  volatile unsigned int array[DIM];
  volatile __shared_write unsigned int head_index;
  volatile __shared_write unsigned int tail_index;
  volatile __shared_write bool is_full;
  volatile __shared_write bool is_empty;  
  volatile __shared_write int turn;
};
typedef volatile struct fifo fifo_t;
       
volatile void* consumer(fifo_t* fifo_array)
{    
  assert(fifo_array);
  
  int count = 0; 
  int i;
  int j;
    
  unsigned int sig = 0;  
  
  while(1)
  {        
    for(i = 0; i < NUM_THREADS; i++)    
    {
      fifo_t* fifo = fifo_array + i;            
      while(fifo->turn == PRODUCER_TURN)
        ;        
    }
     
    //printf("consumer\n"); fflush(stdout);
        
    for(i = 0; i < NUM_THREADS; i++)
    {      
      fifo_t* fifo = fifo_array + i;            
      for(j = 0; j < DIM; j++)
      {          
        assert(fifo->is_empty == false);
        assert(fifo->tail_index < DIM);
              
        sig ^= fifo->array[fifo->tail_index];
        sig += (sig >> 16);
        
        fifo->tail_index = (fifo->tail_index+1) % DIM;
        fifo->is_full = false;
        fifo->is_empty = fifo->head_index == fifo->tail_index ? true : false;
      }                      
    }
        
    for(i = 0; i < NUM_THREADS; i++)    
    {
      fifo_t* fifo = fifo_array + i;            
      fifo->turn = PRODUCER_TURN;        
    }             
    
    count++;
    if(count == NUM_LOOPS)        
      break;                  
  } 
      
  return 0;
}  

volatile void* producer(fifo_t* fifo)
{ 
  assert(fifo);
  
  int i;
  int count = 0;
  int seed = 0x12345678;
    
  while(1)
  {           
    if(fifo->turn != PRODUCER_TURN)
    {      
      continue;        
    } 
    else
    {         
      //printf("producer\n"); fflush(stdout);
      for(i = 0; i < DIM; i++)
      {                  
        seed ^= seed + (seed >> 13) + (count - 543) + count + i;
        assert(fifo->is_full == false);
        assert(fifo->head_index < DIM);        
        fifo->array[fifo->head_index] = seed; // generate a random number       
        fifo->head_index = (fifo->head_index+1) % DIM;
        fifo->is_full = fifo->head_index == fifo->tail_index ? true : false;
        fifo->is_empty = false;   
        fifo->turn = CONSUMER_TURN;
      }     
      
      count++;
      if(count == NUM_LOOPS)        
        break;                               
    }                      
  }     
        
  return 0;
}     
            
int main (int argc, char *argv[])
{  
  int i;
  int j;
  unsigned int seed = 0x1234567;
  
  assert(argc == 3);    
  NUM_THREADS = atoi(argv[1]);
  NUM_LOOPS = atoi(argv[2]);
  
  printf("NUM_THREADS:%d, NUM_LOOPS:%d\n", NUM_THREADS, NUM_LOOPS);
        
  pthread_t* thread_array = NULL;
  thread_array = malloc(sizeof(thread_array[0])*NUM_THREADS);
  
  fifo_t* fifo_array = NULL;
  fifo_array = malloc(sizeof(fifo_array[0])*NUM_THREADS);
  for(i = 0; i < NUM_THREADS; i++)
  {    
    fifo_t* fifo = fifo_array + i;
    for(j = 0; j < DIM; j++)
    {
      seed = seed ^ (seed > 15) + i;
      fifo->array[j] = seed;    
    }
    //memset(fifo->array, 0, sizeof(fifo->array[0])*DIM);
    fifo->head_index = 0;
    fifo->tail_index = 0;
    fifo->is_full = false;
    fifo->is_empty = true;
    fifo->turn = PRODUCER_TURN;    
  }
    
  pthread_attr_t attr;
  int rc;  
  void *status;   
                
        
  /* Initialize and set thread detached attribute */  
  pthread_attr_init(&attr);  
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);   
              
  clock_gettime(CLOCK_REALTIME, &start_time);                
  
  for(i = 0; i < NUM_THREADS; i++)  
    rc = pthread_create(&(thread_array[i]), &attr, producer, fifo_array + i);             
    
  consumer(fifo_array);
  
  for(i = 0; i < NUM_THREADS; i++)  
    pthread_join(&(thread_array[i]), &status);    
             
  clock_gettime(CLOCK_REALTIME, &end_time);                
  
  pthread_attr_destroy(&attr);
     
  printf("Main: program completed. Exiting.\n");  
  printf("s_time.tv_sec:%lld, s_time.tv_nsec:%09lld\n", start_time.tv_sec, start_time.tv_nsec);
  printf("e_time.tv_sec:%lld, e_time.tv_nsec:%09lld\n", end_time.tv_sec, end_time.tv_nsec);
  if(end_time.tv_nsec > start_time.tv_nsec)
  {
    printf("diff_time:%lld.%lld\n", 
      end_time.tv_sec - start_time.tv_sec, 
      end_time.tv_sec - start_time.tv_nsec);
  }
  else
  {
    printf("diff_time:%lld.%lld\n", 
      end_time.tv_sec - start_time.tv_sec - 1, 
      end_time.tv_nsec - start_time.tv_nsec + 1000*1000*1000);
  }
      
  return 0;
}
