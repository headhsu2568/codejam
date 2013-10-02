/******
 * union find tree
 * 
 * example: 
 *   struct huft* tree = huft_init(MAX_N);
 *   int rt = huft_find(tree, x);
 *
 ******/

#ifndef __HEAD_UNION_FIND_TREE__
#define __HEAD_UNION_FIND_TREE__

#include <stdlib.h>

struct huft {
    int* parent;
    int* rank;
};

struct huft* huft_init(int n) {
    int i = 0;
    struct huft* tree = malloc(sizeof(struct huft));
    tree->parent = malloc(sizeof(int)*n);
    tree->rank = malloc(sizeof(int)*n);
    for(; i < n; ++i) {
        tree->parent[i] = i;
        tree->rank[i] = 0;
    }
    return tree;
}

int huft_find(struct huft* tree, int x) {
    if(tree->parent[x] == x) {
        return x;
    }
    else {
        tree->parent[x] = huft_find(tree, tree->parent[x]);
        return tree->parent[x];
    }
}

void huft_unite(struct huft* tree, int x, int y) {
    x = huft_find(tree, x);
    y = huft_find(tree, y);

    if(x == y) return;
    if(tree->rank[x] < tree->rank[y]) {
        tree->parent[x] = y;
    }
    else {
        tree->parent[y] = x;
        if(tree->rank[x] == tree->rank[y]) ++tree->rank[x];
    }
}

int huft_same(struct huft* tree, int x, int y) {
    if(huft_find(tree, x) == huft_find(tree, y)) return 1;
    else return 0;
}

#endif
