TARGET = prime
CC = gcc

#CFLAGS = -g -I ../../Tool/google-perftools-1.7/google/include -L ../../Tool/google-perftools-1.7/google/lib/
CFLAGS = -g

#LIBS = -lm -lprofiler
LIBS = -lm -pg
#LIBS = -lm -lrt
#LIBS = -lm
SRC = prime.c

all: 
	${CC} ${SRC} ${CFLAGS} ${LIBS} -o ${TARGET}
run:
	./${TARGET} 20
clean:
	$(RM) $(TARGET) a.out core *.o
