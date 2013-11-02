CC = gcc
CFLAGS = -g -O0 -lrt
LIBS = -lpthread
SRCS = matrixMultip.c
TARGET = matrixMultip
all:
	gcc ${CFLAGS}  ${LIBS}  ${SRCS} -o ${TARGET}.out 
run:
	./${TARGET}.out 4
clean:
	rm -rf ${TARGET}.out 
