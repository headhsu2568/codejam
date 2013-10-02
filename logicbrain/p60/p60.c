/******
 * LCS (Longest Common Subsequence)
 *
 ******/

#include "../../util/headutil.h"
#include <stdio.h>
#include <stdlib.h>

void solve(int* n, int* m, char* s, char* t);

int main(int argc, char** argv) {
    int expect_argc = 5;
    if(argc != expect_argc) printf("usage: ./p60 <n> <m> <string s> <string t>\n");
    else {
        int n = atoi(argv[1]);
        int m = atoi(argv[2]);
        char* s = argv[3];
        char* t = argv[4];

        printf("Input: \n");
        printf("\tn: %d\n", n);
        printf("\tm: %d\n", m);
        printf("\ts: \"%s\"\n", s);
        printf("\tt: \"%s\"\n\n", t);

        solve(&n, &m, s, t);
    }
    return 0;
}

void solve(int* n, int* m, char* s, char* t) {
    int** dp = NEW2(*n+1, *m+1, int);

    printf("Result:\n");
    int i = 0;
    for(; i < *n; ++i) {
        int j = 0;
        for(; j < *m; ++j) {
            if(s[i] == t[j]) {
                dp[i+1][j+1] = dp[i][j] + 1;
            }
            else {
                dp[i+1][j+1] = MAX(dp[i+1][j], dp[i][j+1]);
            }
        }
    }
    printf("\t%d\n", dp[*n][*m]);

    free(dp);
}
