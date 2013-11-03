#! /bin/sh -
#
# Do we have OpenMP support ?
#

OUTPUT=".test"
CC=gcc
ARGS="-x c -o ${OUTPUT} -fopenmp -"

${CC} ${ARGS} << EOF
#include <stdio.h>

int main()
{
#ifdef _OPENMP
     printf("OpenMP is enabled (%d)!\n", _OPENMP);
#else
     printf("OpenMP is not supported\n");
#endif

     return 0;
}
EOF

./${OUTPUT} && rm ${OUTPUT}
