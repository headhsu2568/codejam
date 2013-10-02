/******
* minimum spanning tree (MST) algorithm: prim
* 
* example:
*   V = [#vertex]
*   cost[V][V] = [cost of each edge]
*   int result = h_prim(V, cost);
*
******/

#ifndef __HEAD_PRIM__
#define __HEAD_PRIM__

#include "headutil.h"
#include <stdlib.h>

#ifndef H_INF
#define H_INF 1 << 30
#endif

int h_prim(int V, int* cost) {
    int mincost = malloc(sizeof(int)*V);
    int used = malloc(sizeof(int)*V);
    int res = 0;
    int i = 0;
    for(; i < V; ++i) {
        mincost[i] = H_INF;
        used[i] = 0;
    }
    mincost[0] = 0;

    while(1) {
        int v = -1;
        for(i = 0; i < V; ++i) {
            if(used[u] == 0 && (v == -1 || mincost[i] < mincost[v])) v = i;
        }
        if(v == -1) break;
        used[v] = 1;
        res += mincost[i];
        for(i = 0; i < V; ++i) {
            mincost[i] = MIN(mincost[i], cost[v][i]);
        }
    }
    return res;
}

#endif
