#include "../../util/headutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void solve(int* n, int* a, int* m, int* k);
void solve2(int* n, int* a, int* m, int* k);

int main(int argc, char** argv) {
    int expect_argc = 5;
    if(argc != expect_argc) printf("usage: ./p66 <n> <{a1, a2, ...}> <{m1, m2, ...}> <k>\n");
    else {
        int n = atoi(argv[1]);
        int k = atoi(argv[4]);
        int* a = malloc(sizeof(int)*n);
        int* m = malloc(sizeof(int)*n);

        printf("Input:\n");
        printf("\tn: %d\n", n);

        const char* delim = "{, }";
        char* pch;
        pch = strtok(argv[2], delim);
        int i = 0;
        printf("\ta: {");
        while(pch != NULL) {
            a[i] = atoi(pch);
            if(i != 0) printf(", ");
            printf("%d", a[i]);
            ++i;
            pch = strtok(NULL, delim);
        }
        printf("}\n");

        pch = strtok(argv[3], delim);
        i = 0;
        printf("\tm: {");
        while(pch != NULL) {
            m[i] = atoi(pch);
            if(i != 0) printf(", ");
            printf("%d", m[i]);
            ++i;
            pch = strtok(NULL, delim);
        }
        printf("}\n");
        printf("\tk: %d\n\n", k);

        printf("Result:\n");
        //solve(&n, a, m, &k);
        solve2(&n, a, m, &k);
    }
    return 0;
}

void solve(int* n, int* a, int* m, int* k) {
    unsigned int** dp = NEW2(*n+1, *k+1, unsigned int);
    dp[0][0] = 1;
    int i = 0;
    for(; i < *n; ++i) {
        int j = 0;
        for(; j <= *k; ++j) {
            int z = 0;
            for(; z <= m[i] && z*a[i] <= j; ++z) {
                //printf("[%d+1][%d] %d | [%d][%d] %d\n",i,j,dp[i+1][j], i,j-z*a[i],dp[i][j-z*a[i]]);
                dp[i+1][j] |= dp[i][j - z*a[i]];
            }
        }
    }
    if(dp[*n][*k] == 1) printf("\tYes\n");
    else printf("\tNo\n");
    free(dp);
}

void solve2(int* n, int* a, int* m, int* k) {
    int dp[*k+1];
    memset(dp, -1, sizeof(dp));
    /*
    int x = 0;
    for(; x <= *n; ++x) {
        int y = 0;
        for(; y <= *k; ++y) {
            dp[y] = -1;
        }
    }
    */
    dp[0] = 0;

    int i = 0;
    for(; i < *n; ++i) {
        int j = 0;
        for(; j <= *k; ++j) {
            //printf("(i:%d) dp[%d]: %d", i, j, dp[j]);
            if(dp[j] >=0) {
                dp[j] = m[i];
            }
            else if(j < a[i] || dp[j - a[i]] <= 0) {
                dp[j] = -1;
            }
            else {
                dp[j] = dp[j -a[i]] - 1;
            }
            //printf(" => %d\n", dp[j]);
        }
    }
    if(dp[*k] >= 0) printf("\tYes\n");
    else printf("\tNo\n");
}
