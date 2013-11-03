CFLAGS = -g -O0 
LDFLAGS = -g -lpthread -lrt -fopenmp

all:							
	gcc $(CFLAGS) -c -o serial_odd_even.o serial_odd_even.c
	gcc $(CFLAGS) -c -o cqueue.o cqueue.c 		
	gcc $(CFLAGS) -c -o main.o main.c
	
	gcc -o main *.o  $(LDFLAGS) 
			
clean:
	rm -rf *.o main	
	rm -rf *.log	

run:
	./main 127 1
