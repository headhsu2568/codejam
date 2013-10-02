/******
 * Roadblocks (POJ 3255)
 *
 ******/

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <queue>
#include "../../util/dijkstra.hpp"

using namespace std;

void solve(vector<struct hedge>* G, int* N, int* R);

int main(int argc, char** argv) {
    int expect_argc = 3;
    if(argc < expect_argc) cout << "usage: ./p113-POJ3255 <N> <R> (from1,to1,cost1) (from2,to2,cost2) ..." << endl;
    else {
        int N = atoi(argv[1]);
        int R = atoi(argv[2]);
        vector<struct hedge>* G = new vector<struct hedge>[N];
        cout << "Input:" << endl;
        cout << "\tN: " << N << endl;
        cout << "\tR: " << R << endl;
        cout << "\tedges: {";
        const char* delim = "(, )";
        int i = 3;
        for(; i < argc; ++i) {
            if(i != 3) cout << ", ";
            int from;
            struct hedge e;
            char* pch = strtok(argv[i], delim);
            from = atoi(pch) - 1;
            pch = strtok(NULL, delim);
            e.to = atoi(pch) - 1;
            pch = strtok(NULL, delim);
            e.cost = atoi(pch);
            G[from].push_back(e);
            cout << "(" << from << ", " << e.to << ", " << e.cost << ")";
        }
        cout << "}\n" << endl;

        solve(G, &N, &R);
    }
    return 0;
}

void solve(vector<struct hedge>* G, int* N, int* R) {
    /* use dijkstra */
    /*
    int** result = h_dijkstra(G, *N, 0);
    vector<int> path = h_get_path(*N, 3, result);
    int i = 0;
    for(; i < path.size(); ++i) {
        cout << path[i] << " ";
    }
    cout << endl;
    */

    priority_queue<H_P, vector<H_P>, greater<H_P> > que;
    int**d = NEW2(*N, 2, int);
    int**d2 = NEW2(*N, 2, int);
    int i = 0;
    for(; i < *N; ++i) {
        d[i][0] = H_INF;
        d2[i][0] = H_INF;
        d[i][1] = -1;
        d2[i][1] = -1;
    }
    d[0][0] = 0;
    d[0][1] = -1;
    d2[0][0] = 0;
    d2[0][1] = -1;
    que.push(H_P(0, 0));

    while(!que.empty()) {
        H_P p =que.top();
        que.pop();
        int v = p.second;
        // current shortest path from start -> v (p.first == d[v][0])
        if(d2[v][0] < p.first) continue;
        for(i = 0; i < G[v].size(); ++i) {
            hedge& e = G[v][i];
            int test = d[v][0] + e.cost;
            int testv = v;
            if(d[e.to][0] > test) {
                int tmp = d[e.to][0];
                int tmp2 = d[e.to][1];
                d[e.to][0] = test;
                d[e.to][1] = v;
                if(d2[e.to][1] == -1) d2[e.to][1] = v;
                test = tmp;
                testv = tmp2;
                que.push(H_P(d[e.to][0], e.to));
            }
            if(d2[e.to][0] > test && d[e.to][0] < test) {
                d2[e.to][0] = test;
                d2[e.to][1] = testv;
                que.push(H_P(d2[e.to][0], e.to));
            }
        }
    }

    cout << "The second shortest distance: " << d2[(*N-1)][0] << endl;
    cout << "The second shortest path: ";
    vector<int> path = h_get_path(*N, 3, d2);
    for(i = 0; i < path.size(); ++i) {
        if(i != 0) cout << " -> ";
        cout << path[i]+1;
    }
    cout << endl;
    return;
}
