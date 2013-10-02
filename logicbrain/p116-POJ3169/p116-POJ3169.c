/******
 * Layout (POJ 3169)
 *
 ******/

#include <stdio.h>
#include <stdlib.h>
#include "../../util/bellman_ford.h"

void solve(int* N, int* ML, int* MD, int* AL, int* BL, int* DL, int* AD, int* BD, int* DD);

int main(int argc, char** argv) {
    int expect_argc = 4;
    if(argc < expect_argc) printf("usage: ./p116-POJ3169 <N> <ML> <MD> (AL1,BL1,DL1) ... (AD1,BD1,DD1) ...\n");
    else {
        int N = atoi(argv[1]);
        int ML = atoi(argv[2]);
        int MD = atoi(argv[3]);
        int* AL = malloc(sizeof(int)*(ML+1));
        int* BL = malloc(sizeof(int)*(ML+1));
        int* DL = malloc(sizeof(int)*(ML+1));
        int* AD = malloc(sizeof(int)*(MD+1));
        int* BD = malloc(sizeof(int)*(MD+1));
        int* DD = malloc(sizeof(int)*(MD+1));
        printf("Input: \n");
        printf("\tN: %d\n", N);
        printf("\tML: %d\n", ML);
        printf("\tMD: %d\n", MD);
        int i = 0;
        printf("\t(AL, BL, DL): {");
        for(; i < ML; ++i) {
            if(i != 0) printf(", ");
            sscanf(argv[4+i], "(%d,%d,%d)", &AL[i], &BL[i], &DL[i]);
            printf("(%d, %d, %d)", AL[i], BL[i], DL[i]);
        }
        printf("}\n");
        printf("\t(AD, BD, DD): {");
        for(i = 0; i < MD; ++i) {
            if(i != 0) printf(", ");
            sscanf(argv[4+ML+i], "(%d,%d,%d)", &AD[i], &BD[i], &DD[i]);
            printf("(%d, %d, %d)", AD[i], BD[i], DD[i]);
        }
        printf("}\n\n");

        solve(&N, &ML, &MD, AL, BL, DL, AD, BD, DD);
    }
    return 0;
}

void solve(int* N, int* ML, int* MD, int* AL, int* BL, int* DL, int* AD, int* BD, int* DD) {
    int E = *N+*ML+*MD-1;
    struct hedge* es = malloc(sizeof(struct hedge)*E);
    int i = 0;
    int j = 0;
    for(; i + 1 < *N; ++i, ++j) {
        es[j].from = i + 1;
        es[j].to = i;
        es[j].cost = 0;
    }
    for(i = 0; i < *ML; ++i, ++j) {
        es[j].from = AL[i]-1;
        es[j].to = BL[i]-1;
        es[j].cost = DL[i];
    }
    for(i = 0; i < *MD; ++i, ++j) {
        es[j].from = BD[i]-1;
        es[j].to = AD[i]-1;
        es[j].cost = -DD[i];
    }
    int** d = h_bellman_ford(es, *N, E, 0);

    int res = d[*N-1][0];
    if(d[0][0] < 0) res = -1;
    else if(res == H_INF) res = -2;
    printf("result: %d (", res);

    for(i = 0; i < *N; ++i) {
        if(i != 0) printf(", ");
        printf("%d", d[i][0]);
    }
    printf(")\n");
    return;
}
