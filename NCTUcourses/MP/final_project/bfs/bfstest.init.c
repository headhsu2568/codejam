// BFSTEST : Test breadth-first search in a graph.
//
// example: cat sample.txt | ./bfstest 1
//
// John R. Gilbert, 17 Feb 2011

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


typedef struct graphstruct { // A graph in compressed-adjacency-list (CSR) form
    int nv;            // number of vertices
    int ne;            // number of edges
    int *nbr;          // array of neighbors of all vertices
    int *firstnbr;     // index in nbr[] of first neighbor of each vtx
} graph;


/* global state */
struct timespec  start_time;
struct timespec  end_time;

unsigned int seed = 0x12345678;
unsigned int myrand(unsigned int *seed, unsigned int input) {
    *seed = (*seed << 13) ^ (*seed >> 15) + input + 0xa174de3;
    return *seed;
};

void sig_check(int *level, int nv) {
    int i;
    unsigned int sig = 0x1234567;

    for(i = 0; i < nv; i++)
    {
        myrand(&sig, level[i]);
    }

    printf("Computed check sum signature:0x%08x\n", sig);
    if(sig == 0x7809978d)
        printf("Result check of sample.txt by signature successful!!\n");
    else if(sig == 0xef872cf0)
        printf("Result check of gen1.pl by signature successful!!\n");
    else if(sig == 0xe61d1d00)
        printf("Result check of gen2.pl by signature successful!!\n");
    else if(sig == 0xee3bc170)
        printf("Result check of gen3.pl by signature successful!!\n");
    else
        printf("Result check by signature failed!!\n");
}


/* Read input from stdio (for genx.pl files, no more than 40 seconds) */
int read_edge_list (int **tailp, int **headp) {
    int max_edges = 100000000;
    int nedges, nr, t, h;

    *tailp = (int *) calloc(max_edges, sizeof(int));
    *headp = (int *) calloc(max_edges, sizeof(int));
    nedges = 0;
    nr = scanf("%i %i",&t,&h);
    while (nr == 2) {
        if (nedges >= max_edges) {
            printf("Limit of %d edges exceeded.\n",max_edges);
            exit(1);
        }
        (*tailp)[nedges] = t;
        (*headp)[nedges++] = h;
        nr = scanf("%i %i",&t,&h);
    }
    return nedges;
}


void print_CSR_graph (graph *G) {
    int vlimit = 20;
    int elimit = 50;
    int e,v;
    printf("\nGraph has %d vertices and %d edges.\n",G->nv,G->ne);
    printf("firstnbr =");
    if (G->nv < vlimit) vlimit = G->nv;
    for (v = 0; v <= vlimit; v++) printf(" %d",G->firstnbr[v]);
    if (G->nv > vlimit) printf(" ...");
    printf("\n");
    printf("nbr =");
    if (G->ne < elimit) elimit = G->ne;
    for (e = 0; e < elimit; e++) printf(" %d",G->nbr[e]);
    if (G->ne > elimit) printf(" ...");
    printf("\n\n");
}


/* Modify the next two functions */
graph * graph_from_edge_list (int *tail, int* head, int nedges) {
    graph *G;
    int i, e, v, maxv;
    G = (graph *) calloc(1, sizeof(graph));
    G->ne = nedges;
    maxv = 0;

    // count vertices
    for (e = 0; e < G->ne; e++) {
        if (tail[e] > maxv) maxv = tail[e];
        if (head[e] > maxv) maxv = head[e];
    }
    G->nv = maxv+1;
    G->nbr = (int *) calloc(G->ne, sizeof(int));
    G->firstnbr = (int *) calloc(G->nv+1, sizeof(int));

    // count neighbors of vertex v in firstnbr[v+1],
    for (e = 0; e < G->ne; e++) G->firstnbr[tail[e]+1]++;

    // cumulative sum of neighbors gives firstnbr[] values
    for (v = 0; v < G->nv; v++) G->firstnbr[v+1] += G->firstnbr[v];

    // pass through edges, slotting each one into the CSR structure
    for (e = 0; e < G->ne; e++) {
        i = G->firstnbr[tail[e]]++;
        G->nbr[i] = head[e];
    }
    // the loop above shifted firstnbr[] left; shift it back right
    for (v = G->nv; v > 0; v--) G->firstnbr[v] = G->firstnbr[v-1];
    G->firstnbr[0] = 0;
    return G;
}


void bfs (int s, graph *G, int **levelp, int *nlevelsp,
        int **levelsizep, int **parentp) {
    int *level, *levelsize, *parent;
    int thislevel;
    int *queue, back, front;
    int i, v, w, e;
    level = *levelp = (int *) calloc(G->nv, sizeof(int));
    levelsize = *levelsizep = (int *) calloc(G->nv, sizeof(int));
    parent = *parentp = (int *) calloc(G->nv, sizeof(int));
    queue = (int *) calloc(G->nv, sizeof(int));

    // initially, queue is empty, all levels and parents are -1
    back = 0;   // position next element will be added to queue
    front = 0;  // position next element will be removed from queue
    for (v = 0; v < G->nv; v++) level[v] = -1;
    for (v = 0; v < G->nv; v++) parent[v] = -1;

    // assign the starting vertex level 0 and put it on the queue to explore
    thislevel = 0;
    level[s] = 0;
    levelsize[0] = 1;
    queue[back++] = s;

    // loop over levels, then over vertices at this level, then over neighbors
    while (levelsize[thislevel] > 0) {
        levelsize[thislevel+1] = 0;
        for (i = 0; i < levelsize[thislevel]; i++) {
            v = queue[front++];       // v is the current vertex to explore from
            for (e = G->firstnbr[v]; e < G->firstnbr[v+1]; e++) {
                w = G->nbr[e];          // w is the current neighbor of v
                if (level[w] == -1) {   // w has not already been reached
                    parent[w] = v;
                    level[w] = thislevel+1;
                    levelsize[thislevel+1]++;
                    queue[back++] = w;    // put w on queue to explore
                }
            }
        }
        thislevel = thislevel+1;
    }
    *nlevelsp = thislevel;
    free(queue);
}


int main (int argc, char* argv[]) {
    graph *G;
    int *level, *levelsize, *parent;
    int *tail, *head;
    int nedges;
    int nlevels;
    int startvtx;
    int i, v, reached;

    if (argc == 2) {
        startvtx = atoi (argv[1]);
    } else {
        printf("usage:   bfstest <startvtx> < <edgelistfile>\n");
        printf("example: cat sample.txt | ./bfstest 1\n");
        exit(1);
    }
    nedges = read_edge_list (&tail, &head);

    clock_gettime(CLOCK_REALTIME, &start_time); //stdio scanf ended, timer starts

    /* You can modify the function below */
    G = graph_from_edge_list (tail, head, nedges);
    free(tail);
    free(head);

    bfs (startvtx, G, &level, &nlevels, &levelsize, &parent);
    /* You can modify the function above */

    clock_gettime(CLOCK_REALTIME, &end_time);  //graph construction and bfs completed timer ends

    print_CSR_graph (G);
    printf("Starting vertex for BFS is %d.\n\n",startvtx);

    reached = 0;
    for (i = 0; i < nlevels; i++) reached += levelsize[i];
    printf("Breadth-first search from vertex %d reached %d levels and %d vertices.\n",
            startvtx, nlevels, reached);
    for (i = 0; i < nlevels; i++) printf("level %d vertices: %d\n", i, levelsize[i]);
    if (G->nv < 20) {
        printf("\n  vertex parent  level\n");
        for (v = 0; v < G->nv; v++) printf("%6d%7d%7d\n", v, parent[v], level[v]);
    }
    printf("\n");

    //printf("sizeof(start_time.tv_sec):%d, sizeof(start_time.tv_nsec):%d\n", sizeof(start_time.tv_sec), sizeof(start_time.tv_nsec));
    //printf("s_time.tv_sec:%ld, s_time.tv_nsec:%ld\n", start_time.tv_sec, start_time.tv_nsec);
    //printf("e_time.tv_sec:%ld, e_time.tv_nsec:%ld\n", end_time.tv_sec, end_time.tv_nsec);
    double execution_time = (double)end_time.tv_sec + (double)end_time.tv_nsec/1000000000.0 - (double)start_time.tv_sec - (double)start_time.tv_nsec/1000000000.0;
    printf("Execution Time:%.4f(s)\n\n", execution_time);

    sig_check(level, G->nv);
    return 0;
}
