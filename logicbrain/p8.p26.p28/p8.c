#include <stdio.h>
#include <stdlib.h>

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

void solve(int* n, int* m, int* k) {
    int f = 0;
    int a, b, c, d;
    for(a = 0; a < *n; ++a) {
        for(b = 0; b < *n; ++b) {
            for(c = 0; c < *n; ++c) {
                for(d = 0; d < *n; ++d) {
                    if(k[a] + k[b] + k[c] + k[d] == *m) {
                        f = 1;
                        printf("\t{%d, %d, %d, %d}\n", k[a], k[b], k[c], k[d]);
                    }
                }
            }
        }
    }

    if(f == 1) printf("Yes\n");
    else printf("No\n");
}
