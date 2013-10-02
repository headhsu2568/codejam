/******
 * shortest path: bellman-ford algorithm
 * 
 * example: 
 *   V = [#vertex]
 *   E = [#edge]
 *   s = [start vertex]
 *   t = [end vertex]
 *   hedge es[E];
 *   int** result = h_bellman_ford(es, V, E, s);
 *   // get path:
 *   int* path = h_get_path(V, t, result);
 *
 ******/

#ifndef __HEAD_BELLMAN_FORD__
#define __HEAD_BELLMAN_FORD__

#ifndef H_INF
#define H_INF 1 << 30
#endif

#include "headutil.h"
#include <stdlib.h>

struct hedge {
    int from;
    int to;
    int cost;
};

int** h_bellman_ford(struct hedge* es, int V, int E, int s) {
    //int* d = malloc(sizeof(int)*V);
    int** d = NEW2(V, 2, int);
    int i = 0;
    for(; i < V; ++i) {
        d[i][0] = H_INF;
        d[i][1] = -1;
    }
    d[s][0] = 0;

    for(i = 0; i < V; ++i) {
        int update = 0;
        int j = 0;
        for(; j < E; ++j) {
            if(d[es[j].from][0] != H_INF && (d[es[j].to][0] > d[es[j].from][0] + es[j].cost)) {
                d[es[j].to][0] = d[es[j].from][0] + es[j].cost;
                d[es[j].to][1]= es[j].from;
                update = 1;
            }
        }
        if(update == 0) break;
    }

    if(i == V) {
        printf("Negative loop occurs\n");
        return NULL;
    }
    else return d;
}

int* h_get_path(int V, int t, int** d) {
    int* path = malloc(sizeof(int)*V);
    int* reverse_path = malloc(sizeof(int)*V);
    int i = 0;
    for(; t != -1; t = d[t][1]) reverse_path[i++] = t;
    --i;

    /* reverse */
    int j = 0;
    for(; i < 0; ++j, --i) {
        path[j] = reverse_path[i];
    }
    return path;
}

#endif
