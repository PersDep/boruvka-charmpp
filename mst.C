#include <algorithm>
#include <vector>
#include <cassert>

#include <error.h>

#include "graph.h"

#include "mst.decl.h"

#include "mst.h"
#include "main.decl.h"

using namespace std;

extern /* readonly */ CProxy_Main mainProxy;


MST::MST(int nVertices, int nEdges, int *edges, double *weights)
{
    EmbeddedEdge *embeddedEdges = (EmbeddedEdge *)edges;
    graph = Graph(nVertices, nEdges, thisIndex, embeddedEdges, weights);
    active = true;
    parent = thisIndex;
}

MST::MST(CkMigrateMessage *msg) {}


void MST::ProcessFragment(int root)
{
    if (!active) {
        int zero = 0;
        const CkCallback cb(CkReductionTarget(Main, reduce), mainProxy);
        contribute(sizeof(int), &zero, CkReduction::max_int, cb);
        return;
    }
    
    parent = root;

    graph.InitCheapestEdges();

    for (int i = 0; i < graph.nEdges; i++)
        if (graph.fragments[parent].count(graph.edges[i].src))
              graph.CheckEdge(graph.Find(graph.edges[i].src), graph.Find(graph.edges[i].dest), i);

    int rank = graph.subsets[parent].rank;
    int cheapestExists = false;
    for (auto &i : graph.fragments[parent])
        if (graph.cheapestEdges[i.first] != -1) {
            cheapestExists = true;
            int root2 = graph.Find(graph.edges[graph.cheapestEdges[i.first]].dest);
            if (rank < graph.subsets[root2].rank || (rank == graph.subsets[root2].rank && parent > root2)) {
                mainProxy.push(graph.edges[graph.cheapestEdges[i.first]].id);
                active = false;
                parent = graph.subsets[root2].parent;
                thisProxy[parent].Receive(graph.fragments[root]);
                break;
            }
        }

    if (graph.fragments[parent].size() == graph.nVertices)
        cheapestExists = true;

    const CkCallback cb(CkReductionTarget(Main, reduce), mainProxy);
    contribute(sizeof(int), &cheapestExists, CkReduction::max_int, cb);
}

void MST::Receive(map<int, bool> fragment)
{
    if (active) {
        thisProxy.PromoteRank(parent);
        for (auto &child : fragment)
            thisProxy.UpdateParent(child.first, parent);
        graph.fragments[parent].insert(fragment.begin(), fragment.end());
    } else {
        thisProxy[parent].Receive(fragment);
    }
}

void MST::UpdateParent(int child, int parent)
{
    graph.subsets[child].parent = parent;
}

void MST::PromoteRank(int parent)
{
    graph.subsets[parent].rank++;
}

#include "mst.def.h"
