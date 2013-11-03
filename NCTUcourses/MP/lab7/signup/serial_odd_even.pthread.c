#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//#define DEBUG
const int RMAX = 1000000000;

typedef struct{
    int tid;
    int n;
    int start;
    int end;
    int* a;
}t_param;

void Usage (char *prog_name);
void Get_args (int argc, char *argv[], int *n_p, char *g_i_p);
void Generate_list (int a[], int n);
void Print_list (int a[], int n, char *title);
void Read_list (int a[], int n);
void Odd_even_sort (int a[], int n);
void Odd_even_iter (int a[], int start, int end, int phase);
void Swap (int *x_p, int *y_p);
void* RunPhase(void* origin_param);

pthread_attr_t attr;
pthread_barrier_t bar;
pthread_t* tid;
t_param* param;

/*-----------------------------------------------------------------*/
/*
   int
   main (int argc, char *argv[])
   {
   int n=16;
   int a[] = {612, 765, 123, 1, 839, 146, 243, 32, 989, 8, 177, 94, 26,83, 15, 4};

   Print_list (a, n, "Before sort");

   Odd_even_sort (a, n);

   Print_list (a, n, "After sort");

   return 0;
   }
   */

/*-----------------------------------------------------------------
 * Function:  Print_list
 * Purpose:   Print the elements in the list
 * In args:   a, n
 */
void Print_list (int a[], int n, char *title)
{
    int i;

    printf ("%s:\n", title);
    for (i = 0; i < n; i++)
        printf ("0x%08x\n", a[i]);
    printf ("\n");
}				/* Print_list */


/*-----------------------------------------------------------------
 * Function:     Odd_even_sort
 * Purpose:      Sort list using odd-even transposition sort
 * In args:      n
 * In/out args:  a
 */
void Odd_even_sort (int a[], int n)
{
    int thread_num=n/2;
    int i;
    
    // initialize thread attribute and barrier
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_barrier_init(&bar, NULL, thread_num);

    // create threads
    tid = (pthread_t*) malloc(sizeof(pthread_t) * thread_num);
    param = (t_param*) malloc(sizeof(t_param) * thread_num);
    for(i=0;i<thread_num;++i){
        param[i].tid=i;
        param[i].n=n;
        param[i].start=1+i*2;
        param[i].end=(n>param[i].start+1)?(param[i].start+2):n;
        param[i].a=a;
        int rc = pthread_create(tid+i, &attr, RunPhase, (void *)&param[i]);
    }

    for(i=0;i<thread_num;++i){
        pthread_join(tid[i],NULL);
    }
}

/*-----------------------------------------------------------------
 * Function:    RunPhase
 * Purpose:     Function for a thread run iteration
 * In args:     origin_param
 */
void* RunPhase(void* origin_param){
#  ifdef DEBUG
    char title[100];
#  endif
    
    t_param* param=(t_param *)origin_param;
    int n=param->n;
    int start=param->start;
    int end=param->end;
    int* a=param->a;
    int phase=0;

    for(phase=0; phase<n; phase++){

        pthread_barrier_wait(&bar);
        //printf("t%d in phase %d(s:%d,e:%d)\n",param->tid,phase,start,end);
        Odd_even_iter(a,start,end,phase);

#     ifdef DEBUG
        sprintf (title, "After phase %d", phase);
        Print_list (a, n, title);
#     endif
    }
}				/* Odd_even_sort */


/*-----------------------------------------------------------------
 * Function:    Odd_even_iter
 * Purpose:     Execute one iteration of odd-even transposition sort
 * In args:     start, end, phase
 * In/out args: a
 */

void Odd_even_iter (int a[], int start, int end, int phase)
{
    int left, right;
    int i=start;

    if (phase % 2 == 0)
    {
        for (i = start; i < end; i += 2)
        {
            left = i - 1;
            if (a[left] > a[i])
                Swap (&a[left], &a[i]);
        }
    }
    else
    {
        for (i = start; i < end - 1; i += 2)
        {
            right = i + 1;
            if (a[i] > a[right])
                Swap (&a[i], &a[right]);
        }
    }
}				/* Odd_even_iter */

/*-----------------------------------------------------------------
 * Function:     Swap
 * Purpose:      Swap contents of x_p and y_p
 * In/out args:  x_p, y_p
 */

void Swap (int *x_p, int *y_p)
{
    int temp = *x_p;
    *x_p = *y_p;
    *y_p = temp;
}				/* Swap */
