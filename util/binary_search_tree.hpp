/******
 * binary search tree
 *
 * example:
 *   hbst_node *root = NULL;
 *   root = hbst_insert(root, 1);
 *   bool result = hbst_find(root, 1);
 *
 ******/

#ifndef __HEAD_BINARY_SEARCH_TREE__
#define __HEAD_BINARY_SEARCH_TREE__

struct hbst_node {
    int val;
    hbst_node *lch, *rch;
};

hbst_node *hbst_insert(hbst_node *p, int x) {
    if(p == NULL) {
        hbst_node *q = new hbst_node;
        q->val = x;
        q->lch = q->rch = NULL;
        return q;
    }
    else {
        if(x < p->val) p->lch = hbst_insert(p->lch, x);
        else p->rch = hbst_insert(p->rch, x);
        return p;
    }
}

bool hbst_find(hbst_node *p, int x) {
    if(p == NULL) return NULL;
    else if(x == p->val) return true;
    else if(x < p->val) return hbst_find(p->lch, x);
    else return hbst_find(p->rch, x);
}

hbst_node *remove(hbst_node *p, int x) {
    if(p == NULL) return NULL;
    else if(x < p->val) p->lch = remove(p->lch, x);
    else if(x > p->val) p->rch = remove(p->rch, x);
    else if(p->lch == NULL) {
        hbst_node *q = p->rch;
        delete p;
        return q;
    }
    else if(p->lch->rch == NULL) {
        hbst_node *q = p->lch;
        q->rch = p->rch;
        delete p;
        return q;
    }
    else {
        hbst_node *p;
        for(q = p->lch; q->rch->rch != NULL; q = q->rch);
        hbst_node *r = q->rch;
        q->rch = r->lch;
        r->lch = p->lch;
        r->rch = p->rch;
        delete p;
        return r;
    }
    return p;
}

#endif
