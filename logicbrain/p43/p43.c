#include "../../util/headutil.h"
#include <stdio.h>
#include <stdlib.h>

void solve(int* C, int* A);

const int V[6] = {1 ,5 ,10, 50, 100, 500};

int main(int argc, char** argv) {
    int expect_argc = 8;
    if(argc != expect_argc) printf("usage: ./p43 <c1> <c5> <c10> <c50> <c100> <c500> <A>\n");
    else {
        int C[6];
        C[0] = atoi(argv[1]);
        C[1] = atoi(argv[2]);
        C[2] = atoi(argv[3]);
        C[3] = atoi(argv[4]);
        C[4] = atoi(argv[5]);
        C[5] = atoi(argv[6]);
        int A = atoi(argv[7]);

        printf("Input:\n");
        printf("\tc1: %d\n", C[0]);
        printf("\tc5: %d\n", C[1]);
        printf("\tc10: %d\n", C[2]);
        printf("\tc50: %d\n", C[3]);
        printf("\tc100: %d\n", C[4]);
        printf("\tc500: %d\n", C[5]);
        printf("\tA: %d\n", A);

        solve(C, &A);
    }
    return 0;
}

void solve(int* C, int* A) {
    int ans = 0;

    int i = 5;
    for(; i >= 0; --i) {
        int t = MIN(*A/V[i], C[i]);
        *A -= t*V[i];
        ans += t;
    }
    printf("%d\n", ans);

    return;
}
