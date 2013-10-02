/******
 * unlimited knapsack
 *
 ******/

#include "../../util/headutil.h"
#include <stdio.h>
#include <stdlib.h>

void solve(int* n, int* W, int* w, int* v);
void solve2(int* n, int* W, int* w, int* v);

int main(int argc, char** argv) {
    int expect_argc = 4;
    if(argc < expect_argc) printf("usage: ./p62 <n> <W> <w1,v1> <(w2,v2)> <...>\n");
    else {
        int n = atoi(argv[1]);
        int W = atoi(argv[2]);
        int* w = malloc(sizeof(int)*(n+1));
        int* v = malloc(sizeof(int)*(n+1));
        printf("Input: \n");
        printf("\tn: %d\n", n);
        printf("\tW: %d\n", W);
        printf("\t(wi, vi): {");
        int i = 0;
        for(; i < n; ++i) {
            if(i != 0) printf(", ");
            sscanf(argv[3+i], "(%d,%d)", &w[i], &v[i]);
            printf("(%d, %d)", w[i], v[i]);
        }
        printf("}\n\n");

        //solve(&n, &W, w, v);
        solve2(&n, &W, w, v);
    }
    return 0;
}

void solve(int* n, int* W, int* w, int* v) {
    int** dp = NEW2(*n+1, *W+1, int);
    printf("Result:\n");
    int i = 0;
    for(; i < *n; ++i) {
        int j = 0;
        for(; j <= *W; ++j) {
            int k = 0;
            for(; k*w[i] <= j; ++k) {
                dp[i+1][j] = MAX(dp[i+1][j], dp[i][j-k*w[i]]+k*v[i]);
            }
        }
    }
    printf("\t%d\n", dp[*n][*W]);
    free(dp);
}

void solve2(int* n, int* W, int* w, int* v) {
    /*** better performance: O(nW) ***/

    int** dp = NEW2(*n+1, *W+1, int);
    printf("Result:\n");
    int i = 0;
    for(; i < *n; ++i) {
        int j = 0;
        for(; j <= *W; ++j) {
            if(j < w[i]) dp[i+1][j] = dp[i][j];
            else {
                dp[i+1][j] = MAX(dp[i][j], dp[i+1][j-w[i]]+v[i]);
            }
        }
    }
    printf("\t%d\n", dp[*n][*W]);
    free(dp);
}
