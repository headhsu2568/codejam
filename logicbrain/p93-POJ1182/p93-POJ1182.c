/******
 * Food-Chain (POJ 1182)
 *
 ******/

#include "../../util/union_find_tree.h"
#include <stdio.h>
#include <stdlib.h>

void solve(int* N, int* K, int* T, int* X, int* Y);

int main(int argc, char** argv) {
    int expect_argc = 3;
    if(argc < expect_argc) printf("usage: ./p93-POJ1182 <N> <K> <(T1,X1,Y1)> <T2,X2,Y2> ...\n");
    else {
        int N = atoi(argv[1]);
        int K = atoi(argv[2]);
        int* T = malloc(sizeof(int)*K);
        int* X = malloc(sizeof(int)*K);
        int* Y = malloc(sizeof(int)*K);
        printf("Input: \n");
        printf("\tN: %d\n", N);
        printf("\tK: %d\n", K);
        printf("\t(Ti, Xi, Yi): {");
        int i = 0;
        for(; i < K; ++i) {
            if(i != 0) printf(", ");
            sscanf(argv[3+i], "(%d,%d,%d)", &T[i], &X[i], &Y[i]);
            printf("(%d, %d, %d)", T[i], X[i], Y[i]);
        }
        printf("}\n\n");

        solve(&N, &K, T, X, Y);
    }
    return 0;
}

void solve(int* N, int* K, int* T, int* X, int* Y) {
    struct huft* tree = huft_init((*N) * 3);

    int ans = 0;
    int i = 0;
    for(; i < *K; ++i) {
        int t = T[i];

        /* (A, B, C) : (x, x+N, x+N*2) & (y, y+N, y+N*2) */
        int x = X[i] - 1;
        int y = Y[i] - 1;

        if(x < 0 || y < 0 || *N <= x || *N <= y) {
            ++ans;
            continue;
        }

        if(t == 1) {
            if(huft_same(tree, x, y+*N) | huft_same(tree, x, y+*N*2)) ++ans;
            else {
                huft_unite(tree, x, y);
                huft_unite(tree, x+*N, y+*N);
                huft_unite(tree, x+*N*2, y+*N*2);
            }
        }
        else {
            if(huft_same(tree, x, y) || huft_same(tree, x, y+*N*2)) ++ans;
            else {
                huft_unite(tree, x, y+*N);
                huft_unite(tree, x+*N, y+*N*2);
                huft_unite(tree, x+*N*2, y);
            }
        }
    }

    printf("result: %d\n", ans);
}
