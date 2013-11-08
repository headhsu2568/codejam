CC = gcc
CFLAGS = -g -O3
LIBS = -fopenmp -lrt -lm -lpthread -pg
SRCS = bfstest.c
TARGET = bfstest
all:
	gcc ${CFLAGS}  ${LIBS}  ${SRCS} -o ${TARGET}
clean:
	rm -rf ${TARGET}.out ${TARGET} 
test:
	cat sample.txt  | ./bfstest 1
run1:
	cat gen1.txt | ./bfstest 1
run2:
	cat gen2.txt | ./bfstest 1
run3:
	cat gen3.txt | ./bfstest 1

