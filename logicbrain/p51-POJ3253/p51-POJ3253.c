/******
 * Fence Repair (POJ 3253)
 *
 ******/

#include <stdio.h>
#include <stdlib.h>

void solve(int N, int* L);

int main(int argc, char** argv) {
    int expect_argc = 3;
    if(argc < expect_argc) printf("usage: ./p51-POJ3253 <N> <L1> <L2> ...\n");
    else {
        int N = atoi(argv[1]);
        int* L = malloc(sizeof(int)*(N+1));
        printf("Input: \n");
        printf("\tN: %d\n", N);
        printf("\tL: {");
        int i = 0;
        for(; i < N; ++i) {
            if(i != 0) printf(", ");
            L[i] = atoi(argv[i+2]);
            printf("%d", L[i]);
        }
        printf("}\n\n");

        solve(N, L);
    }
    return 0;
}

void solve(int N, int* L) {
    long long ans = 0;

    while(N > 1) {
        int mii1 = 0;
        int mii2 = 1;
        if(L[mii1] > L[mii2]) {
            mii1 = 1;
            mii2 = 0;
        }

        int i = 2;
        for(; i < N; ++i) {
            if(L[i] < L[mii1]) {
                mii2 = mii1;
                mii1 = i;
            }
            else if(L[i] < L[mii2]) mii2 = i;
        }

        int t = L[mii1] + L[mii2];
        ans += t;

        if(mii1 == N - 1) {
            mii1 = mii2;
            mii2 = N - 1;
        }
        L[mii1] = t;
        L[mii2] = L[N - 1];
        --N;
    }

    printf("Result: %lld\n", ans);
    return;
}
