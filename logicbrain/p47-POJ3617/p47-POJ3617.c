/******
 * Best Cow Line (POJ 3617)
 *
 ******/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void solve(int* N, char* S);

int main(int argc, char** argv) {
    int expect_argc = 3;
    if(argc != expect_argc) printf("usage: ./p47-POJ3617 <N> <S>\n");
    else {
        int N = atoi(argv[1]);
        char* S = malloc(sizeof(char)*(N+1));
        strncpy(S, argv[2], N);
        printf("Input: \n");
        printf("\tN: %d\n", N);
        printf("\tS: %s\n", S);
        solve(&N, S);
    }
    return 0;
}

void solve(int* N, char* S) {
    int a = 0;
    int b = *N - 1;

    printf("\nResult: \n\tT: ");
    while(a <= b) {
        int left = 0;
        int i = 0;
        for(; a + 1 <= b; ++i) {
            if(S[a+i] < S[b-i]) {
                left = 1;
                break;
            }
            else if(S[a+i] > S[b-i]) {
                left = 0;
                break;
            }
        }
        if(left == 1) putchar(S[a++]);
        else putchar(S[b--]);
    }
    putchar('\n');
    return;
}
