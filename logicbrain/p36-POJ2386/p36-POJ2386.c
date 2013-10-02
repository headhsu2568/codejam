/******
 * Lake Counting (POJ 2386)
 *
 ******/

#include <stdio.h>
#include <stdlib.h>

void solve(int* N, int* M, char** field);
void dfs(int* N, int* M, char** field, int x, int y);

int main(int argc, char** argv) {
    if(argc != 4) printf("usage: ./p36-POJ2386 <N> <M> <field>\n");
    else {
        int N = atoi(argv[1]);
        int M = atoi(argv[2]);
        char** field = malloc(sizeof(char*)*N);

        printf("Input:\n");
        printf("\tN: %d\n", N);
        printf("\tM: %d\n", M);
        printf("\tfield:\n");
        int i = 0;
        for(; i < N*M; ++i) {
            if(i%M == 0) {
                field[i/M] = malloc(sizeof(char)*M);
                printf("\t\t");
            }
            field[i/M][i%M] = argv[3][i];
            printf("%c", argv[3][i]);
            if(i%M == M-1) printf("\n");
        }

        solve(&N, &M, field);

        free(field);
    }
}

void solve(int* N, int* M, char** field) {
    int res = 0;
    int i = 0;
    for(; i < *N; ++i) {
        int j = 0;
        for(; j < *M; ++j) {
            if(field[i][j] == 'W') {
                dfs(N, M, field, i, j);
                ++res;
            }
        }
    }
    printf("result: %d\n", res);
}

void dfs(int* N, int* M, char** field, int x, int y) {
    field[x][y] = '.';

    int dx = -1;
    for(; dx <= 1; ++dx) {
        int dy = -1;
        for(; dy <=1; ++dy) {
            int nx = x + dx;
            int ny = y + dy;

            if(0 <= nx && nx < *N && 0 <= ny && ny < *M && field[nx][ny] == 'W') dfs(N, M, field, nx, ny);
        }
    }
    return;
}
