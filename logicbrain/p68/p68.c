/******
 * LIS (Longest Increasing Subsequence)
 *
 ******/

#include "../../util/headutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void solve(int* n, int* a);

int main(int argc, char** argv) {
    int expect_argc = 3;
    if(argc != expect_argc) printf("usage: ./p68 <n> <{a1, a2, ...}>\n");
    else {
        int n = atoi(argv[1]);
        int* a = malloc(sizeof(int)*n);
        printf("Input:\n");
        printf("\tn: %d\n", n);
        printf("\ta: {");
        int i = 0;
        char* delim = "{, }";
        char* pch = strtok(argv[2], delim);
        while(pch != NULL) {
            a[i] = atoi(pch);
            if(i != 0) printf(", ");
            printf("%d", a[i]);
            ++i;
            pch = strtok(NULL, delim);
        }
        printf("}\n\n");

        solve(&n, a);
    }
    return 0;
}

void solve(int* n, int* a) {
    int* dp = malloc(sizeof(int)*(*n));
    int res = 0;
    int i = 0;
    printf("Result:\n");
    for(; i < *n; ++i) {
        dp[i] = 1;
        int j = 0;
        for(; j < i; ++j) {
            if(a[j] < a[i]) {
                dp[i] = MAX(dp[i], dp[j] + 1);
            }
        }
        res = MAX(dp[i], res);
    }
    printf("\t%d\n", res);
}
