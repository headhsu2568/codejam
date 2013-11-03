TARGET = prime
CC = gcc

CFLAGS = -g -fopenmp 

LIBS = -lm -lrt
SRC = prime.c

all: 
	${CC} ${SRC} ${CFLAGS} ${LIBS} -o ${TARGET}.out
run:
	./${TARGET} 20
clean:
	$(RM) *.out core *.o
