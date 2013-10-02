/******
 * Saruman's Army (POJ 3069)
 *
 ******/

#include <stdio.h>
#include <stdlib.h>

void solve(int* N, int* R, int* X);

int main(int argc, char** argv) {
    int expect_argc = 4;
    if(argc < expect_argc) printf("usage: ./p49-POJ3069 <N> <R> <X1> <X2> ...\n");
    else {
        int N = atoi(argv[1]);
        int R = atoi(argv[2]);
        int* X = malloc(sizeof(int)*(N+1));
        int i = 0;
        printf("Input: \n");
        printf("\tN: %d\n", N);
        printf("\tR: %d\n", R);
        printf("\tX: {");
        for(; i < N; ++i) {
            if(i != 0) printf(", ");
            X[i] = atoi(argv[i+3]);
            printf("%d", X[i]);
        }
        printf("}\n\n");

        solve(&N, &R, X);
    }
    return 0;
}

void solve(int* N, int* R, int* X) {
    //sort(X, X + *N);

    int i = 0;
    int ans = 0;

    while(i < *N) {
        int s = X[i];
        ++i;

        while(i < *N && (X[i] <= s + *R)) ++i;
        int p = X[i - 1];
        printf("Tag point: %d\n", p);

        while(i < *N && (X[i] <= p + *R)) ++i;

        ++ans;
    }
    printf("Result: %d\n", ans);
    return;
}
