#include <algorithm>
#include <vector>
#include <cassert>

#include <error.h>

#include "deferrment.h"

#include "hello.decl.h"

#include "hello.h"
#include "main.decl.h"

using namespace std;

extern /* readonly */ CProxy_Main mainProxy;


Hello::Hello() {}

Hello::Hello(CkMigrateMessage *msg) {}


void Hello::ProcessFragment(int nVertices, int nEdges, int parent, int sizeEdges, int *edges, int sizeWeights, double *weights, int sizeSubsets, int *subsets, int sizeFragment, int *fragment)
{
    // CkPrintf("Running fragment %d on chare %d\n", parent, thisIndex);
    Graph graph;
    graph.nVertices = nVertices;
    graph.nEdges = nEdges;
    EmbeddedEdge *myEdges = (EmbeddedEdge *)edges;
    Subset *mySubsets = (Subset *)subsets;
    for (int i = 0; i < sizeEdges; i++)
      graph.edges.push_back(Edge(myEdges[i].id, myEdges[i].src, myEdges[i].dest, weights[i]));
    graph.subsets = vector<Subset>(mySubsets, mySubsets + sizeSubsets);
    graph.cheapestEdges = vector<int>(graph.nVertices, -1);
    for (int i = 0; i < sizeFragment; i++)
      graph.fragments[parent][fragment[i]] = true;

    for (int i = 0; i < graph.nEdges; i++)
        if (graph.fragments[parent].count(graph.edges[i].src))
              graph.CheckEdge(graph.Find(graph.edges[i].src), graph.Find(graph.edges[i].dest), i);

    const CkCallback cb(CkReductionTarget(Main, reduce), mainProxy);
    // CkPrintf("Time to contribute\n");
    contribute(sizeof(int) * graph.cheapestEdges.size(), graph.cheapestEdges.data(), CkReduction::max_int, cb);
    // CkPrintf("Contributed\n");
}


#include "hello.def.h"
