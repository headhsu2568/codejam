#include <stdio.h>
#include <stdlib.h>

int dfs(int* n, int *k, int* a, int i, int sum);

int main(int argc, char** argv) {
    if(argc < 4) printf("usage: ./p34 <n> <k> <a1, a2, ...>\n");
    else {
        int n = atoi(argv[1]);
        int k = atoi(argv[2]);
        int* a = malloc(sizeof(int)*n);
        int i = 0;
        printf("Input:\n");
        printf("\tn: %d\n", n);
        printf("\tk: %d\n", k);
        printf("\ta: { ");
        for(; i < n; ++i) {
            a[i] = atoi(argv[3+i]);
            printf("%d ", a[i]);
        }
        printf("}\n\n");

        int result = dfs(&n, &k, a, 0, 0);

        if(result == 1) printf("Yes\n");
        else printf("No\n");
        free(a);
    }
    return 0;
}

int dfs(int* n, int *k, int* a, int i, int sum) {
    if(i == *n) return sum == *k;
    if(dfs(n, k, a, i + 1, sum)) return 1;
    if(dfs(n, k, a, i + 1, sum + a[i])) return 1;
    return 0;
}
