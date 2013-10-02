/******
* minimum spanning tree (MST) algorithm: kruskal
* 
* example:
*   V = [#vertex]
*   E = [#edges]
*   hedge* es[E];
*   int result = h_kruskal(V, E, es);
*
******/

#ifndef __HEAD_KRUSKAL__
#define __HEAD_KRUSKAL__

#include <algorithm>
#include "headutil.hpp"
#include "union_find_tree.hpp"

#ifndef H_INF
#define H_INF 1 << 30
#endif

using namespace std;

struct hedge {
    int from;
    int to;
    int cost;
};

bool h_comp(const hedge& e1, const hedge& e2) {
    return e1.cost < e2.cost;
}

int h_kruskal(int V, int E, struct hedge* es) {
    sort(es, es + E, h_comp);
    struct huft* tree = huft_init(V);
    int res = 0;
    int i = 0;
    for(; i < E; ++i) {
        if(!huft_same(tree, es[i].from, es[i].to)) {
            huft_unite(tree, es[i].from, es[i].to);
            res += es[i].cost;
        }
    }
    return res;
}

#endif
