CC = gcc
CFLAGS = -g -O0
LIBS = -lpthread -lrt
#LIBS = -lpthread -lrt
#SRCS = piMultithread.c 
SRCS = pi.c
TARGET = pi
all:
	gcc ${CFLAGS}  ${LIBS}  ${SRCS} -o ${TARGET}.out 
run:
	./${TARGET}.out 20
clean:
	rm -rf ${TARGET}.out 
