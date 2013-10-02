/******
 * shortest path: dijkstra algorithm
 * 
 * example: 
 *   G[V][V] = [each edge & cost]
 *   V = [#vertex]
 *   s = [start vertex]
 *   t = [end vertex]
 *   int* result = h_dijkstra(G, V, s);
 *   // get path:
 *   vector<int> path = h_get_path(V, t, result);
 *
 ******/

#ifndef __HEAD_DIJKSTRA__
#define __HEAD_DIJKSTRA__

#ifndef H_INF
#define H_INF 1 << 30
#endif

#include "headutil.hpp"
#include <algorithm>
#include <queue>

using namespace std;

struct hedge {
    int to;
    int cost;
};

/* pair<shortest, vertex> */
typedef pair<int, int> H_P;

int** h_dijkstra(vector<struct hedge>* G, int V, int s) {
    priority_queue<H_P, vector<H_P>, greater<H_P> > que;
    //int* d = new int[V];
    int**d = NEW2(V, 2, int);
    int i = 0;
    for(; i < V; ++i) {
        d[i][0] = H_INF;
        d[i][1] = -1;
    }
    d[s][0] = 0;
    que.push(H_P(0, s));
    while(!que.empty()) {
        H_P p = que.top();
        que.pop();
        int v = p.second;
        if(d[v][0] < p.first) continue;
        for(i = 0; i < G[v].size(); ++i) {
            if(d[G[v][i].to][0] > d[v][0] + G[v][i].cost) {
                d[G[v][i].to][0] = d[v][0] + G[v][i].cost;
                d[G[v][i].to][1] = v;
                que.push(H_P(d[G[v][i].to][0], G[v][i].to));
            }
        }
    }
    return d;
}

vector<int> h_get_path(int V, int t, int** d) {
    vector<int> path;
    for(; t != -1; t = d[t][1]) path.push_back(t);
    reverse(path.begin(), path.end());
    return path;
}

#endif
