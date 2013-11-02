PROG = matmul
CFLAGS = -g -lrt
#CFLAGS = -g -funroll-all-loops
#CFLAGS = -g -lpthread

.SUFFIXES: .o .c

.c.o: 
	gcc $(CFLAGS) -c $*.c

$(PROG): $(PROG).o
#	gcc -O1 -o $(PROG) $(CFLAGS)  $(PROG).o
	gcc -o $(PROG) $(CFLAGS)  $(PROG).o



clean:
	rm -f *.o $(PROG) *~ 
