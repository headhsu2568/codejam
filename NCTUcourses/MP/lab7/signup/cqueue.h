#ifndef _JJR_CQUEUE_H_
#define _JJR_CQUEUE_H_

#define bool char
#define false 0
#define true (!0)

typedef struct cqueue cqueue_t;
struct cqueue
{        
    unsigned int m_size;  
    unsigned int m_num;  
    unsigned int m_count;  
    void* m_array;    
    unsigned int m_head;
    unsigned int m_tail;
    bool m_isfull;
    bool m_isempty;
}; 

void cqueue_init(cqueue_t* cqueue, int size, int num);
void cqueue_push_back(cqueue_t* cqueue, void* entry);  
void cqueue_pop_front(cqueue_t* cqueue);         
void* cqueue_front(cqueue_t* cqueue);   
unsigned int cqueue_count(cqueue_t* cqueue);   

#endif // _JJR_CQUEUE_H_
