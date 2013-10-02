/******
 * Crazy Row (2009 Round2 A)
 *
 ******/

#include <stdio.h>
#include <stdlib.h>
#include "../../util/headutil.h"

int** solve(int *N, int** M);
void printM(int *N, int** M);

int main(int argc, char** argv) {
    int expect_argc = 2;
    if(argc <= expect_argc) printf("usage: ./p121-2009_round2_A <N> <row 1> <row 2> ... <row N>\n");
    else {
        int N = atoi(argv[1]);
        int** M = NEW2(N, N, int);
        printf("Input: \n");
        printf("\tN: %d\n", N);
        printf("\tM:");
        int i = 0;
        for(; i < N; ++i) {
            int j = 0;
            printf("\n\t\t");
            for(; j < N; ++j) {
                M[i][j] = argv[i+2][j]-48;
                printf("%d  ", M[i][j]);
            }
        }
        printf("\n\n");

        int** M_new = solve(&N, M);
    }
    return 0;
}

int** solve(int* N, int** M) {
    int** M_new = NEW2(*N, *N, int);
    int res = 0;
    int i = 0;
    int* pos = malloc(sizeof(int)*(*N));
    for(; i < *N; ++i) {
        pos[i] = -1;
        int j = 0;
        for(; j < *N; ++j) {
            if(M[i][j] == 1) pos[i] = j;
        }
    }

    for(i = 0; i < *N; ++i) {
        int j = 0;
        int target = -1;
        for(j = i; j < *N; ++j) {
            if(pos[j] <= i) {
                target = j;
                break;
            }
        }

        //printf("select '%d' to '%d'\n", target, i);

        for(j = target; j > i; --j) {
            int tmp = pos[j];
            pos[j] = pos[j-1];
            pos[j-1] = tmp;
            int* tmp_ptr = M[j];
            M[j] = M[j-1];
            M[j-1] = tmp_ptr;
            ++res;
        }

        //if(j != target) printM(N, M);
    }
    printf("result: %d\n", res);
    printM(N, M);
    return M;
}

void printM(int *N, int** M) {
    int i = 0;
    for(; i < *N; ++i) {
        int j = 0;
        printf("\n\t\t");
        for(; j < *N; ++j) {
            printf("%d  ", M[i][j]);
        }
    }
    printf("\n");
}
