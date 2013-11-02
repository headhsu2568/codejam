TARGET = prime
CFLAGS = -g3
CC = gcc
LIBS = -lm
SRC = prime.c
all: 
	${CC} ${SRC} ${CFLAGS} ${LIBS} -o ${TARGET}
run:
	./${TARGET} 10
clean:
	rm -f $(TARGET) a.out core *.o
