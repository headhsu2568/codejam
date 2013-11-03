#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

#include "cqueue.h"

typedef struct signupsheet signupsheet_t;
struct signupsheet
{
    int* array; // the signature array
    pthread_mutex_t mutex;
};

int student_num = 0;
pthread_t* thread_array = NULL;
int* golden = NULL;

pthread_barrier_t bar;

cqueue_t cqueue;

pthread_mutex_t mutex;

int sheet_num = 1;
signupsheet_t* sheet_array = 0;
signupsheet_t final_sheet;

void delay()
{
    int i;
    int count = rand() & 0xff;
    for(i = 0; i < count; i++)
        pthread_yield();
}

int compare(const void *arg1, const void *arg2)
{
    return (*(int*) arg1 - *(int *) arg2);
}

void check_result()
{
    int rc;
    qsort((void *)golden, student_num, sizeof(int), compare);
    // qsort((void *)final_sheet.array, student_num, sizeof(int), compare);

    rc = memcmp(golden, final_sheet.array, sizeof(final_sheet.array[0])*student_num);

    if(rc == 0)
        printf ("Check Ok!!!\n");
    else
        printf ("Check Error:%d!!!\n", rc);
}

void* student(void * arg)
{
    int i, j;
    int student_id = (int) arg;
    int signup_count = 0;
    int signup_index = 0;
    signupsheet_t* sheet = NULL;

    // wait for all sudent sitting in the room
    pthread_barrier_wait(&bar);

    pthread_mutex_lock(&mutex);
    // get a sheet
    while(sheet == NULL)
    {
        if(cqueue.m_isempty != true)
        {
            sheet = *((signupsheet_t**)(cqueue_front(&cqueue)));
            cqueue_pop_front(&cqueue);
            assert(sheet);
        }
    }
    assert(sheet);

    // cacluate what the total number of student that had signed
    for(i = 0; i < student_num; i++)
    {
        delay(); // you can't remove this
        if(sheet->array[i] != 0)
            signup_index++;
    }
    sheet->array[signup_index] = student_id;

    // return its sheet
    assert(cqueue.m_isfull != true);
    cqueue_push_back(&cqueue, &sheet);


    // caculate the sum of signup student
    for(i = 0; i < sheet_num; i++)
    {
        sheet = sheet_array + i;
        for(j = 0; j < student_num; j++)
        {
            if(sheet->array[j] != 0)
            {
                signup_count++;
            }
        }
    }

    pthread_mutex_unlock(&mutex);

    //printf("Student ID:[%08x]: signup_index:%d, signup_count:%d\n", student_id, signup_index, signup_count);
    //fflush(stdout);
}

int main (int argc, char *argv[])
{
    int i, j, rc;
    pthread_attr_t attr;

    assert(argc == 3);
    student_num = atoi(argv[1]);
    sheet_num = atoi(argv[2]);
    assert(student_num > 0);
    assert(sheet_num > 0);

    srand(time(NULL));

    cqueue_init(&cqueue, sizeof(sheet_array), sheet_num);

    // Initialize and set thread detached attribute
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&mutex, NULL);

    // prepare sheet_array
    sheet_array = malloc(sizeof(sheet_array[0])*sheet_num);
    assert(sheet_array);
    for(i = 0; i < sheet_num; i++)
    {
        sheet_array[i].array = malloc(sizeof(sheet_array[0].array[0])*student_num);
        assert(sheet_array[i].array);
        memset(sheet_array[i].array, 0, sizeof(sheet_array[0].array[0])*student_num);
        pthread_mutex_init(&(sheet_array[i].mutex), NULL);
        signupsheet_t* psheet = sheet_array + i;
        cqueue_push_back(&cqueue, &psheet);
    }

    // prepare final_sheet
    final_sheet.array = malloc(sizeof(int)*student_num);
    memset(final_sheet.array, 0, sizeof(final_sheet.array[0])*student_num);
    pthread_mutex_init(&(final_sheet.mutex), NULL);

    // prepare the golden data for check()
    golden = malloc(sizeof(int)*student_num);
    assert(golden);

    pthread_barrier_init(&bar, NULL, student_num);


    // initial thread creation
    thread_array = (pthread_t*) malloc(sizeof(pthread_t) * student_num);
    assert(thread_array);
    for(i = 0; i < student_num; i++)
    {
        int student_id = 0;
        while(student_id == 0)
            student_id = rand();
        assert(student_id != 0);

        int rc = pthread_create(thread_array+i, &attr, student, (void *)student_id);
        assert(rc == 0);
        golden[i] = student_id;
    }

    for(i = 0; i < student_num; i++)
        pthread_join(thread_array[i], NULL);

    // sort all student autograph
    int signup_count = 0;
    for(i = 0; i < sheet_num; i++)
        for(j = 0; j < student_num; j++)
            if(sheet_array[i].array[j])
                final_sheet.array[signup_count++] = sheet_array[i].array[j];

    printf("signup_count:%d\n", signup_count);
    assert(signup_count == student_num);

    Odd_even_sort(final_sheet.array, student_num);
    //Print_list(final_sheet.array, student_num,
    //"Student autograph Booki (Sorted)");

    check_result();

    return 0;
}
