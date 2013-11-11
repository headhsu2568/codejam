#include <stdio.h>
#define MAX_SIZE 92 

unsigned long memo[MAX_SIZE];

unsigned long fib_recursive(unsigned int n) {
    if(n <= 1) return n;
    else if(n <= MAX_SIZE && memo[n] != 0) return memo[n];
    else return (memo[n] = fib_recursive(n-1) + fib_recursive(n-2));
}

unsigned long fib_loop(unsigned int n) {
    if(n <= 1) return n;
    unsigned long a = 0;
    unsigned long b = 1;
    for(; n > 2; --n) {
        if(a < b) a = a + b;
        else b = a + b;
    }
    return a + b;
}

unsigned long main() {
    unsigned int n = MAX_SIZE;
    printf("Fibonacci(%d) is %ld\n", n, fib_recursive(n));
    printf("Fibonacci(%d) is %ld\n", n, fib_loop(n));
    return 0;
}
