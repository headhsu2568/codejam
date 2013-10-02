/******
 * 0-1 knapsack
 *
 ******/

#include "../../util/headutil.h"
#include <stdio.h>
#include <stdlib.h>

void solve(int* n, int* W, int* w, int* v);
int rec(int* n, int* w, int* v, int**dp, int i, int j);
void solve_dp(int* n, int* W, int* w, int* v);
void solve_dp2(int* n, int* W, int* w, int* v);
void solve_dp3(int* n, int* W, int* w, int* v);

int main(int argc, char** argv) {
    int expect_argc = 4;
    if(argc < expect_argc) printf("usage: ./p55 <n> <W> <(w1,v1)> <(w2,v2)> <...>\n");
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
        //solve_dp(&n, &W, w, v);
        //solve_dp2(&n, &W, w, v);
        solve_dp3(&n, &W, w, v);
    }
    return 0;
}

void solve(int* n, int* W, int* w, int* v) {
    int** dp = NEW2(*n+1, *W+1, int);

    /* initialize */
    //memset(dp, -1, sizeof(int)*(*n)*(*W));
    int i = 0;
    for(; i <= *n; ++i) {
        int j = 0;
        for(; j <= *W; ++j) {
            dp[i][j] = -1;
        }
    }

    int ans = rec(n, w, v, dp, 0, *W);
    printf("Result: %d\n", ans);
    free(dp);
}

int rec(int* n, int* w, int* v, int** dp, int i, int j) {
    if(dp[i][j] >= 0) {
        return dp[i][j];
    }
    int res;
    if(i == *n) {
        res = 0;
    }
    else if(j < w[i]) {
        res = rec(n, w, v, dp, i+1, j);
    }
    else {
        res = MAX(rec(n, w, v, dp, i+1, j), rec(n, w, v, dp, i+1, j-w[i])+v[i]);
    }
    dp[i][j] = res;
    return res;
}

void solve_dp(int* n, int* W, int* w, int* v) {
    int** dp = NEW2(*n+1, *W+1, int);

    int i = *n-1;
    for(; i >= 0; --i) {
        int j = 0;
        for(; j <= *W; ++j) {
            if(j < w[i]) dp[i][j] = dp[i+1][j];
            else dp[i][j] = MAX(dp[i+1][j], dp[i+1][j-w[i]] + v[i]);
        }
    }
    printf("Result: %d\n", dp[0][*W]);
    free(dp);
}

void solve_dp2(int* n, int* W, int* w, int* v) {
    int** dp = NEW2(*n+1, *W+1, int);

    int k = 0;
    for(; k <= *W; ++k) {
        dp[0][k] = 0;
    }

    int i = 0;
    for(; i < *n; ++i) {
        int j = 0;
        for(; j <= *W; ++j) {
            if(j < w[i]) dp[i+1][j] = dp[i][j];
            else {
                dp[i+1][j] = MAX(dp[i][j], dp[i][j-w[i]]+v[i]);
            }
        }
    }

    printf("Result: %d\n", dp[*n][*W]);
    free(dp);
}

void solve_dp3(int* n, int* W, int* w, int* v) {
    int** dp = NEW2(*n+1, *W+1, int);

    int k = 0;
    for(; k <= *W; ++k) {
        dp[0][k] = 0;
    }

    int i = 0;
    for(; i < *n; ++i) {
        int j = 0;
        for(; j <= *W; ++j) {
            dp[i+1][j] = MAX(dp[i+1][j], dp[i][j]);
            if(j < w[i] && j + w[i] <= *W) {
                dp[i+1][j+w[i]] = MAX(dp[i+1][j+w[i]], dp[i][j]+v[i]);
            }
        }
    }

    printf("Result: %d\n", dp[*n][*W]);
    free(dp);
}
