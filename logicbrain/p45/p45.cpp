#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;

void solve(int* N, pair<int, int>* itv);

int main(int argc, char** argv) {
    int expect_argc = 4;
    if(argc != expect_argc) cout << "usage: ./p45 <N> <S> <T>" << endl;
    else {
        int N = atoi(argv[1]);
        int* S = new int[N];
        int* T = new int[N];
        pair<int, int>* itv = new pair<int, int>[N];

        const char* delim = ", ";
        char* pch;

        pch = strtok(argv[2], delim);
        int i = 0;
        while(pch != NULL) {
            S[i] = atoi(pch);
            ++i;
            pch = strtok(NULL, delim);
        }

        pch = strtok(argv[3], delim);
        i = 0;
        while(pch != NULL) {
            T[i] = atoi(pch);
            ++i;
            pch = strtok(NULL, delim);
        }

        for(int i = 0; i < N; ++i) {
            itv[i].first = T[i];    // terminate time
            itv[i].second = S[i];   // start time
        }

        solve(&N, itv);
    }
    return 0;
}

void solve(int* N, pair<int, int>* itv) {
    //sort(itv, itv + *N);

    int ans = 0;
    int t = 0;

    for(int i = 0; i < *N; ++i) {
        if(t < itv[i].second) {
            ++ans;
            t = itv[i].first;
        }
    }

    cout << ans << endl;
    return;
}
