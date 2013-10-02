#include <iostream>
#include <cstdlib>
#include <cstring>
#include <queue>

using namespace std;

void solve(vector<int>* G, int* V);
bool dfs(vector<int>* G, int* color, int v, int c);

int main(int argc, char** argv) {
    int expect_argc = 2;
    if(argc < expect_argc) cout << "usage: ./p102 <n> <(vi1,vj1)> <(vi2, vj2)> ..." << endl;
    else {
        int V = atoi(argv[1]);
        vector<int>* G = new vector<int>[V];
        cout << "Input:" << endl;
        cout << "\tn: " << V << endl;
        cout << "\tedges: {";
        const char* delim = "(, )";
        int i = 2;
        for(; i < argc; ++i) {
            if(i != 2) cout << ", ";
            int x;
            int y;
            char* pch = strtok(argv[i], delim);
            x = atoi(pch);
            pch = strtok(NULL, delim);
            y = atoi(pch);
            G[x].push_back(y);
            G[y].push_back(x);
            cout << "(" << x << ", " << y << ")";
        }
        cout << "}\n" << endl;

        solve(G, &V);
    }
    return 0;
}

void solve(vector<int>* G, int* V) {
    int* color = new int[*V];
    int i = 0;
    for(; i < *V; ++i) {
        if(color[i] == 0) {
            /* uncolor */

            if(!dfs(G, color, i, 1)) {
                cout << "No" << endl;
                return;
            }
        }
    }
    cout << "Yes" << endl;
}

bool dfs(vector<int>* G, int* color, int v, int c) {
    color[v] = c;
    int i = 0;
    for(; i < G[v].size(); ++i) {
        if(color[G[v][i]] == c) return false;
        if(color[G[v][i]] == 0 && !dfs(G, color, G[v][i], -c)) return false;
    }
    return true;
}
