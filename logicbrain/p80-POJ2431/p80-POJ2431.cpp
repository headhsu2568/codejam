/******
 * Expedition (POJ 2431)
 *
 ******/

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <queue>

using namespace std;

void solve(int* N, int* L, int* P, int* A, int* B);

int main(int argc, char** argv) {
    int expect_argc = 6;
    if(argc != expect_argc) cout << "usage: ./p80-POJ2431 <N> <L> <P> <{A1, A2, ...}> <{B1, B2, ...}>" << endl;
    else {
        int N = atoi(argv[1]);
        int L = atoi(argv[2]);
        int P = atoi(argv[3]);
        int* A = new int[N + 1];
        int* B = new int[N + 1];
        cout << "Input:" << endl;
        cout << "\tN: " << N << endl;
        cout << "\tL: " << L << endl;
        cout << "\tP: " << P << endl;
        char* delim = "{, }";
        char* pch = strtok(argv[4], delim);
        int i = 0;
        cout << "\tA: {";
        while(pch != NULL) {
            A[i] = atoi(pch);
            if(i != 0) cout << ", ";
            cout << A[i];
            ++i;
            pch = strtok(NULL, delim);
        }
        cout << "}" << endl;

        pch = strtok(argv[5], delim);
        i = 0;
        cout << "\tB: {";
        while(pch != NULL) {
            B[i] = atoi(pch);
            if(i != 0) cout << ", ";
            cout << B[i];
            ++i;
            pch = strtok(NULL, delim);
        }
        cout << "}\n" << endl;

        solve(&N, &L, &P, A, B);

        delete B;
        delete A;
    }
    return 0;
}

void solve(int* N, int* L, int* P, int* A, int* B) {
    cout << "Result:" << endl;

    A[*N] = *L;
    B[*N] = 0;
    *N = *N + 1;

    priority_queue<int> que;
    int ans = 0, pos = 0, tank = *P;
    int i = 0;
    for(; i <= *N; ++i) {
        int distance = A[i] - pos;
        while(tank - distance < 0) {
            if(que.empty()) {
                ans = -1;
                cout << "\t" << ans << endl;
                return;
            }
            tank += que.top();
            que.pop();
            ++ans;
        }
        tank -= distance;
        pos = A[i];
        que.push(B[i]);
    }
    cout << "\t" << ans << endl;
}
