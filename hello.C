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


Hello::Hello()
{
    working = true;
}

Hello::Hello(CkMigrateMessage *msg) {}


void Hello::ProcessFragment(int nVertices, int nEdges, int _parent, int sizeEdges, int *edges, int sizeWeights, double *weights, int sizeSubsets, int *subsets, int sizeFragment, int *fragment)
{
    if (!working) {
        int zero = 0;
        const CkCallback cb(CkReductionTarget(Main, reduce), mainProxy);
        contribute(sizeof(int), &zero, CkReduction::max_int, cb);
        return;
    }
    
    parent = _parent;
    // CkPrintf("Running fragment %d on chare %d on proc %d\n", parent, thisIndex, CkMyPe());
    if (!graph.nVertices) { 
      graph.nVertices = nVertices;
      graph.nEdges = nEdges;
      EmbeddedEdge *myEdges = (EmbeddedEdge *)edges;
      for (int i = 0; i < sizeEdges; i++)
        graph.edges.push_back(Edge(myEdges[i].id, myEdges[i].src, myEdges[i].dest, weights[i]));

      Subset *mySubsets = (Subset *)subsets;
      graph.subsets = vector<Subset>(mySubsets, mySubsets + sizeSubsets);
      graph.fragments.clear();
      for (int i = 0; i < sizeFragment; i++)
        graph.fragments[parent][fragment[i]] = true;
    }

    // CkPrintf("Running fragment %d of size %u on chare %d on proc %d\n", parent, graph.fragments[parent].size(), thisIndex, CkMyPe());
    // if (graph.fragments[parent].size() == nVertices)
    //   mainProxy.done();

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
                // CkPrintf("Finishing fragment %d on chare %d on proc %d\n", parent, thisIndex, CkMyPe());
                working = false;
                parent = graph.subsets[root2].parent;
                thisProxy[graph.subsets[root2].parent].Receive(graph.fragments[_parent], graph.edges[graph.cheapestEdges[i.first]].id);
                break;
            }
        }

    if (graph.fragments[parent].size() == nVertices)
        cheapestExists = true;

    // CkPrintf("Time to contribute\n");
    const CkCallback cb(CkReductionTarget(Main, reduce), mainProxy);
    contribute(sizeof(int), &cheapestExists, CkReduction::max_int, cb);
    // contribute(sizeof(int) * graph.cheapestEdges.size(), graph.cheapestEdges.data(), CkReduction::max_int, cb);
    // CkPrintf("Contributed\n");
}

void Hello::Receive(map<int, bool> fragment, int id)
{;
    // CkPrintf("Receive fragment for parent %d on chare %d on proc %d\n", parent, thisIndex, CkMyPe());
    if (working) {
        for (auto &child : fragment) {
            // graph.subsets[child.first].parent = parent;
            thisProxy.UpdateParent(child.first, parent);
        }
        graph.fragments[parent].insert(fragment.begin(), fragment.end());
        thisProxy.PromoteRank(parent);
        mainProxy.push(id);
        // CkPrintf("Receive fragment for parent %d on chare %d on proc %d\n", parent, thisIndex, CkMyPe());
        // graph.PrintFragments();
    } else {
        thisProxy[parent].Receive(fragment, id);
    }
}

void Hello::UpdateParent(int child, int parent)
{
    graph.subsets[child].parent = parent;
}

void Hello::PromoteRank(int parent)
{
    graph.subsets[parent].rank++;
}

#include "hello.def.h"
