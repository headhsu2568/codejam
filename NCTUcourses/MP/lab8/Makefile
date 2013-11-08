CFLAGS = -g -O0 
LDFLAGS = -g -lpthread -lrt -fopenmp

all:									
	gcc $(CFLAGS) -c -o main.o main.c
	
	gcc -o main *.o  $(LDFLAGS) 
			
clean:
	rm -rf *.o main	
	rm -rf *.log	

run:
	./main 8 128 1000000