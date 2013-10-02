// include guard
#ifndef __HEAD_UTIL__
#define __HEAD_UTIL__

#ifndef NEW2
#define NEW2(X,Y,TYPE) (TYPE**)new2(X,Y,sizeof(TYPE))
#include <stdlib.h>
void* new2(int x, int y, int size) {
    register int i;
    void** ptr;
    ptr = (void**)malloc(x*y*size + x*sizeof(void*)); // s*sizeof(void*) is the space of the 1st pointers
    for(i = 0; i < x; ++i) {
        ptr[i] = ((char*)(ptr + x)) + i*y*size;
    }
    return ptr;
}
#endif

#ifndef MIN
#define MIN(A,B) \
    ({ \
     __typeof__ (A) _A = (A); \
     __typeof__ (B) _B = (B); \
     _A < _B ? _A : _B; \
     })
#endif

#ifndef MAX
#define MAX(A,B) \
    ({ \
     __typeof__ (A) _A = (A); \
     __typeof__ (B) _B = (B); \
     _A > _B ? _A : _B; \
     })
#endif

#endif
