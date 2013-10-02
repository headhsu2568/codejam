#include <stdio.h>
#include <stdlib.h>

void sort(int* x, int* xn);
int binary_search(int* k, int* n, int x);
void solve(int* n, int* m, int* k);

int main(int argc, char** argv) {
    if(argc < 4) printf("usage: ./p8 <n> <m> <k1, k2, ...>\n");
    else {
        int n = atoi(argv[1]);
        int m = atoi(argv[2]);
        int* k = malloc(sizeof(int)*n);
        int i = 0;
        printf("Input:\n");
        printf("\tn: %d\n", n);
        printf("\tm: %d\n", m);
        printf("\tk: { ");
        for(; i < n; ++i) {
            k[i] = atoi(argv[3+i]);
            printf("%d ", k[i]);
        }
        printf("}\n\n");

        solve(&n, &m, k);
        free(k);
    }
    return 0;
}

void sort(int* x, int *xn) {
    /*** ignore ***/
}

int binary_search(int* k, int* n, int x) {
    int a = 0, b = *n;

    while(b - a >= 1) {
        int i = (a + b) / 2;
        if(k[i] == x) return i;
        else if(k[i] < x) a = i + 1;
        else b =i;
    }

    return -1;
}

void solve(int* n, int* m, int* k) {
    sort(k, k+*n);
    int f = 0;
    int a, b, c, d;
    for(a = 0; a < *n; ++a) {
        for(b = 0; b < *n; ++b) {
            for(c = 0; c < *n; ++c) {

                /*** using binary search ***/
                if((d = binary_search(k, n, *m - k[a] - k[b] - k[c])) >= 0) {
                    f = 1;
                    printf("\t{%d, %d, %d, %d}\n", k[a], k[b], k[c], k[d]);
                }
            }
        }
    }

    if(f == 1) printf("Yes\n");
    else printf("No\n");
}
