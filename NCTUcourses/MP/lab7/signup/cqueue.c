#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "cqueue.h"

void cqueue_init(cqueue_t* cqueue, int size, int num)
{    
    assert(cqueue);
    assert(size > 0);
    assert(num > 0);

    cqueue->m_size = size;  
    cqueue->m_num = num;  
    cqueue->m_count = 0;
    cqueue->m_head = 0;
    cqueue->m_tail = 0;
    cqueue->m_isfull = false;
    cqueue->m_isempty = true;

    cqueue->m_array = malloc(size*num);;    
    assert(cqueue->m_array);
} 

unsigned int cqueue_count(cqueue_t* cqueue)
{
    assert(cqueue);
    assert(cqueue->m_array);
    return cqueue->m_count;    
};   

void* cqueue_front(cqueue_t* cqueue)
{
    assert(cqueue);
    assert(cqueue->m_array);  
    assert(cqueue->m_tail < cqueue->m_num);  
    assert(cqueue->m_count);

    if(cqueue->m_count)    
        return cqueue->m_array + cqueue->m_tail*cqueue->m_size;    
    else
        return NULL;
} 

void cqueue_push_back(cqueue_t* cqueue, void* entry)
{
    assert(cqueue);
    assert(entry);
    assert(cqueue->m_array);
    assert(cqueue->m_isfull == false);
    assert(cqueue->m_head < cqueue->m_num);

    memcpy(cqueue->m_array + cqueue->m_head*cqueue->m_size, entry, cqueue->m_size);      

    cqueue->m_head = (cqueue->m_head+1) % cqueue->m_num;        
    cqueue->m_count++;
    cqueue->m_isfull = cqueue->m_head == cqueue->m_tail ? true : false;      
    cqueue->m_isempty = false;  

    assert(cqueue->m_count >= 0 && cqueue->m_count <= cqueue->m_num);  
} 

void cqueue_pop_front(cqueue_t* cqueue)
{
    assert(cqueue);
    assert(cqueue->m_array);
    assert(cqueue->m_isempty == false);
    assert(cqueue->m_tail < cqueue->m_num);

    //void* entry = cqueue->m_array + cqueue->m_tail*cqueue->m_size;  
    cqueue->m_tail = (cqueue->m_tail+1) % cqueue->m_num;        
    cqueue->m_count--;  
    cqueue->m_isempty = cqueue->m_head == cqueue->m_tail ? true : false;      
    cqueue->m_isfull = false;  

    assert(cqueue->m_count >= 0 && cqueue->m_count <= cqueue->m_num);
} 

void cqueue_content(cqueue_t* cqueue)
{    
    assert(cqueue);
    unsigned int m_size;  
    unsigned int m_num;  
    unsigned int m_count;  
    void* m_array;    
    unsigned int m_head;
    unsigned int m_tail;
    bool m_isfull;
    bool m_isempty;


    printf("size:%u, num:%u, count:%u ,head:%u, tail:%u, isfull:%d, isempty:%d\n",
            cqueue->m_size, 
            cqueue->m_num, 
            cqueue->m_count,
            cqueue->m_head, 
            cqueue->m_tail,
            cqueue->m_isfull,
            cqueue->m_isempty);
} 

/*
//int cqueue_test()
int main()
{
cqueue_t cqueue;
cqueue_init(&cqueue, sizeof(int), 4);

int push_count = 0;
int pop_count = 0;

while(pop_count < 100)
{
if(cqueue.m_isfull == false)
{
if(rand() & 1)
{
printf("%d push ", push_count);
cqueue_push_back(&cqueue, &push_count);
int* ptr = cqueue_front(&cqueue);
//printf("%08x, %d\n", ptr, *ptr);
push_count++;
cqueue_content(&cqueue);
}
}

if(cqueue.m_isempty == false)    
{      
if(rand() & 1)
{        
printf("%d pop ", pop_count);
int* ptr = cqueue_front(&cqueue);  
//printf("ptr:%08x, %d\n", ptr, *ptr);         
pop_count++;
cqueue_pop_front(&cqueue);        
cqueue_content(&cqueue);

fflush(stdout);
assert(*ptr == pop_count-1);  
}
}
}
return 0;
}
*/
