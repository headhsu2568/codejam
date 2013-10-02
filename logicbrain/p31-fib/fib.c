#include <stdio.h>
#include <stdlib.h>

unsigned int fib(int n);
int* memo;

int main(int argc, char** argv) {
    if(argc == 2) {
        int n = atoi(argv[1]);
        memo = malloc(sizeof(unsigned int)*n+1);
        printf("Input: %d\n", n);
        fib(n);
        int i = 0;
        for(; i < n; ++i) {
            printf("%u ", memo[i]);
        }
        printf("\n");
        free(memo);
    }
    else printf("usage: ./fib <number>\n");
}

unsigned int fib(int n) {
    if(n <= 1) return n;
    else if(memo[n] != 0) return memo[n];
    else return memo[n] = fib(n-1) + fib(n-2);
}
