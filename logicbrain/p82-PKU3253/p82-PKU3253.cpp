/******
 * Fence Repair (PKU 3253)
 *
 ******/

#include <iostream>
#include <cstdlib>
#include <queue>

using namespace std;

typedef long long ll;

void solve(int* N, int* L);

int main(int argc, char** argv) {
    int expect_argc = 3;
    if(argc < expect_argc) cout << "usage: ./p82-PKU3253 <N> <L1> <L2> ..." << endl;
    else {
        int N = atoi(argv[1]);
        int* L = new int[N+1];
        cout << "Input:" << endl;
        cout << "\tN: " << N << endl;
        cout << "\tL: {";
        int i = 0;
        for(; i < N; ++i) {
            if(i != 0) cout << ", ";
            L[i] = atoi(argv[i+2]);
            cout << L[i];
        }
        cout << "}\n" << endl;

        solve(&N, L);
    }
    return 0;
}

void solve(int* N, int* L) {
    cout << "Result:" << endl;
    ll ans = 0;

    priority_queue<int, vector<int>, greater<int> > que;
    int i = 0;
    for(; i < *N; ++i) {
        que.push(L[i]);
    }

    while(que.size() > 1) {
        int min1, min2;

        min1 = que.top();
        que.pop();
        min2 = que.top();
        que.pop();

        ans += min1 + min2;
        que.push(min1 + min2);
    }

    cout << "\t" << ans << endl;
}
