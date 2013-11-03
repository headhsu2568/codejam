all:
	gcc -c -O0 -g main.c
	gcc -o main *.o -lpthread -lrt
	
run:
	./main 2 1000
	
clean:
	rm -rf *.o main	*.s *.ss
