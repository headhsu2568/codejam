/***
 * 計算 n! 最後有幾個連續 0
 *
 ***/
#include <stdio.h>
#include <stdlib.h>
#define N 50

int continuous_zero_bad(int n) {
    int i = 1;
    int z = 0;
    long count = 1;
    for(; i <= n; ++i) {
        count = count * i;
        //printf("%ld ", count);
        while(count % 10 == 0) {
            count = count / 10;
            ++z;
            //printf("%ld(%d) ", count, z);
        }
        //printf("\n");
    }
    return z;
}

int continuous_zero(int n) {
    int i = 1;
    int z = 0;
    int test;
    for(; i <= n; ++i) {
        test = i;
        while(test % 5 == 0) {
            ++z;
            test = test / 5;
        }
    }
    return z;
}

int main() {
    int n = N;

    /*
    int z = continuous_zero_bad(n);
    printf("%d! has %d continuous zeros from continuous_zero_bad\n", n, z);
    */

    int z = continuous_zero(n);
    printf("%d! has %d continuous zeros from continuous_zero\n", n, z);
    return 0;
}
