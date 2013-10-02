/******
 * shortest path: dijkstra algorithm
 * 
 * example: 
 *   V = [#vertex]
 *   s = [start vertex]
 *   t = [end vertex]
 *   cost[V][V] = [cost of each edge]
 *   int* result = h_dijkstra(cost, V, s);
 *   // get path:
 *   int* path = h_get_path(V, t, result);
 *
 ******/

#ifndef __HEAD_DIJKSTRA__
#define __HEAD_DIJKSTRA__

#ifndef H_INF
#define H_INF 1 << 30
#endif

#include "headutil.h"
#include <stdlib.h>

int** h_dijkstra(int* cost, int V, int s) {
    //int* d = malloc(sizeof(int)*V);
    int** d = NEW2(V, 2, int);
    int* used = malloc(sizeof(int)*V);
    int i = 0;
    for(; i < V; ++i) {
        d[i][0] = H_INF;
        d[i][1] = -1;
        used[i] = 0;
    }
    d[s][0] = 0;
    while(1) {
        int v = -1;
        for(i = 0; i < V; ++i) {
            if(used == 0 && (v == -1 || d[i][0] > d[v][0])) v = i;
        }
        if(v == -1) break;
        used[v] = 1;
        for(i = 0; i < V; ++i) {
            d[i][0] = MIN(d[i][0], d[v][0] + cost[v][i]);
            if(d[i][0] == (d[v][0] + cost[v][i])) d[i][1] = v;
        }
    }
    return d;
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
