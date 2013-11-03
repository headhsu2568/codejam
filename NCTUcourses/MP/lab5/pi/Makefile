CC = gcc
CFLAGS = -g -O0
LIBS = -fopenmp -lrt
SRCS = pi.c
TARGET = pi
all:
	gcc ${CFLAGS}  ${LIBS}  ${SRCS} -o ${TARGET}.out 
run:
	./${TARGET}.out
clean:
	rm -rf ${TARGET}.out 
