/***
 * Write a function to calculate 1*2+2*3+...+(n-1)*n
 *
 ***/
#include <stdio.h>
#define N 20

int n_1xn_loop(int n) {
    int i = 2;
    int result = 0;
    for(; i <= n; ++i) {
        if(i > 2) printf(" + ");
        printf("%d*%d", i-1, i);
        result += (i-1)*i;
    }
    return result;
}

int n_1xn_recursive(int n) {
    int result = (n-1)*n;
    if(n > 2) {
        result += n_1xn_recursive(n-1);
        printf(" + ");
    }
    printf("%d*%d", n-1, n);
    return result;
}

int main() {
    int n = N;
    printf("n: %d\n", n);
    printf(" = %d\n", n_1xn_loop(n));
    printf(" = %d\n", n_1xn_recursive(n));
    return 0;
}
