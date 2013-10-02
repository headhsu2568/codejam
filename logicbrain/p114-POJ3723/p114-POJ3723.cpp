/******
 * Conscription (POJ 3723)
 *
 ******/

#include <iostream>
#include <cstdlib>
#include <cstring>
#include "../../util/kruskal.hpp"

using namespace std;

void solve(int& N, int& M, int& R, struct hedge* es);

int main(int argc, char** argv) {
    int expect_argc = 4;
    if(argc < expect_argc) cout << "usage: ./p114-POJ3723 <N> <M> <R> (x1,y1,d1) (x1,y2,d2) ..." << endl;
    else {
        int N = atoi(argv[1]);
        int M = atoi(argv[2]);
        int R = atoi(argv[3]);
        cout << "Input:\n" << endl;
        cout << "\tN: " << N << endl;
        cout << "\tM: " << M << endl;
        cout << "\tR: " << R << endl;
        cout << "\trelationship: {";
        struct hedge* es = new struct hedge[R];
        const char* delim = "(, )";
        int i = 0;
        for(; i < R; ++i) {
            if(i != 0) cout << ", ";
            char* pch = strtok(argv[i+4], delim);
            es[i].from = atoi(pch);
            pch = strtok(NULL, delim);
            es[i].to = N + atoi(pch);
            pch = strtok(NULL, delim);
            es[i].cost = 0-atoi(pch);
            cout << "(" << es[i].from << ", " << es[i].to << ", " << 0-es[i].cost << ")";
        }
        cout << "}\n" << endl;

        solve(N, M, R, es);
    }
    return 0;
}

void solve(int& N, int& M, int& R, struct hedge* es) {
    int res = h_kruskal(N+M, R, es);
    res += 10000 * (N + M);
    cout << "min cost: " << res << endl;
}
