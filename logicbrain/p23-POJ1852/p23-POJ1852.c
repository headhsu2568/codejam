/******
 * Ants (POJ 1852)
 *
 ******/

#include <stdio.h>
#include <stdlib.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void solve(int*, int*, int*);

int main(int argc, char** argv) {
    int L;
    int n;
    int* x;
    if(argc < 4) printf("usage: ./p23-POJ1852 <L> <n> <{x1, x2, ...}>\n");
    else {
        L = atoi(argv[1]);
        n = atoi(argv[2]);
        x = malloc(sizeof(int)*n);
        printf("Input: \n");
        printf("\tL: %d\n", L);
        printf("\tn: %d\n", n);
        printf("\tx: { ");
        int i = 0;
        for(; i < n; ++i) {
            x[i] = atoi(argv[3+i]);
            printf("%d ", x[i]);
        }
        printf("}\n");
        solve(&L, &n, x);
        free(x);
    }
    return 0;
}

void solve(int* L, int* n, int* x) {
    int minT = 0;
    int i = 0;

    /*** calculate the minimum time ***/
    for(; i < *n; ++i) {
        minT = MAX(minT, MIN(x[i], *L - x[i]));
    }

    int maxT = 0;

    /*** calculate the maximum time ***/
    for(i = 0; i < *n; ++i) {
        maxT = MAX(maxT, MAX(x[i], *L - x[i]));
    }

    printf("Minimum: %d\nMaximum: %d\n", minT, maxT);
}
