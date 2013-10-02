#include "../../util/headutil.hpp"
#include <iostream>
#include <queue>
#include <cstdlib>

using namespace std;

void solve(int* N, int* M, char** maze, int* sx, int* sy, int* gx, int* gy);
int bfs(int* N, int* M, char** maze, int* sx, int* sy, int* gx, int* gy);

const int INF = 100000000;
typedef pair<int, int> P;

int main(int argc, char** argv) {
    int expect_argc = 4;
    if(argc < expect_argc) cout << "usage: ./p38 <N> <M> <maze>" << endl;
    else {
        int N = atoi(argv[1]);
        int M = atoi(argv[2]);
        char** maze = NEW2(N, M, char);
        int sx, sy, gx, gy;

        cout << "Input:" << endl;
        cout << "\tN: " << N << endl;
        cout << "\tM: " << M << endl;
        cout << "\tmaze: " << endl;
        int i = 0;
        for(; i < N*M; ++i) {
            if(i%M == 0) cout << "\t\t";
            maze[i/M][i%M] = argv[3][i];
            if(argv[3][i] == 'S') {
                sx = i/M;
                sy = i%M;
            }
            else if(argv[3][i] == 'G') {
                gx = i/M;
                gy = i%M;
            }
            cout << argv[3][i];
            if(i%M == M-1) cout << endl;
        }

        solve(&N, &M, maze, &sx, &sy, &gx, &gy);
        delete []maze;
    }
    return 0;
}

void solve(int* N, int* M, char** maze, int* sx, int* sy, int* gx, int* gy) {
    int res = 0;
    res = bfs(N, M, maze, sx, sy, gx, gy);
    cout << "result: " << res << endl;
    return;
}

int bfs(int* N, int* M, char** maze, int* sx, int* sy, int* gx, int* gy) {
    int** d = NEW2(*N, *M, int);
    int dx[4] = {1, 0, -1, 0}, dy[4] = {0, 1, 0, -1};
    queue<P> que;
    int goal = INF;

    for(int i = 0; i < *N; ++i) {
        for(int j = 0; j < *M; ++j) {
            d[i][j] = INF;
        }
    }
    que.push(P(*sx, *sy));
    d[*sx][*sy] = 0;

    while(que.size()) {
        P p = que.front();
        que.pop();
        if(p.first == *gx && p.second == *gy) break;

        for(int i = 0; i < 4; ++i) {
            int nx = p.first + dx[i];
            int ny = p.second + dy[i];

            if(0 <= nx && nx < *N && 0 <= ny && ny < *M && maze[nx][ny] != '#' && d[nx][ny] == INF) {
                que.push(P(nx, ny));
                d[nx][ny] = d[p.first][p.second] + 1;
            }
        }
    }
    goal =  d[*gx][*gy];

    delete []d;
    return goal;
}
