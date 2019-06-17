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


MST::MST(int nVertices, int nEdges)
{
    graph = Graph(nVertices, nEdges, thisIndex);
    active = true;
    parent = thisIndex;
}

MST::MST(CkMigrateMessage *msg) {}


void MST::ProcessFragment(int root, int size, int *edges, double *weights)
{
    if (!active) {
        thisProxy[root].ckDestroy();
        return;
    }
    
    parent = root;

    if (size) {
        EmbeddedEdge *embeddedEdges = (EmbeddedEdge *)edges;
        for (int i = 0; i < size; i++)
            graph.edges.push_back(Edge(embeddedEdges[i].id, embeddedEdges[i].src, embeddedEdges[i].dest, weights[i]));
    } else {
        vector<vector<Edge>::iterator> edgesToErase;
        for (auto edge = graph.edges.begin(); edge != graph.edges.end(); edge++)
            if (graph.Find(edge->src) == parent && graph.Find(edge->dest) == parent)
                edgesToErase.push_back(edge);
        for (auto it = edgesToErase.rbegin(); it != edgesToErase.rend(); it++) {
            graph.edges.erase(*it);
        }
    }

    vector<EmbeddedEdge> embeddedEdges;
    vector<double> embeddedWeights;
    for (auto &edge : graph.edges) {
        embeddedEdges.push_back(EmbeddedEdge(edge));
        embeddedWeights.push_back(edge.weight);
    }

    graph.InitCheapestEdges();

    for (size_t i = 0; i < graph.edges.size(); i++)
        graph.CheckEdge(graph.Find(graph.edges[i].src), graph.Find(graph.edges[i].dest), int(i));

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
                thisProxy[parent].Receive(graph.fragments[root], int(graph.edges.size()), (int *)embeddedEdges.data(), embeddedWeights.data(), root);
                break;
            }
        }

    if (graph.fragments[parent].size() == graph.nVertices)
        cheapestExists = true;

    if (active) {
        const CkCallback cb(CkReductionTarget(Main, reduce), mainProxy);
        contribute(sizeof(int), &cheapestExists, CkReduction::max_int, cb);
    }
}

void MST::Receive(map<int, bool> fragment, int size, int *edges, double *weights, int author)
{
    if (active) {
        thisProxy.PromoteRank(parent);
        for (auto &child : fragment)
            thisProxy.UpdateParent(child.first, parent);
        graph.fragments[parent].insert(fragment.begin(), fragment.end());
        EmbeddedEdge *embeddedEdges = (EmbeddedEdge *)edges;
        for (int i = 0; i < size; i++)
            graph.edges.push_back(Edge(embeddedEdges[i].id, embeddedEdges[i].src, embeddedEdges[i].dest, weights[i]));
        thisProxy[author].Answer();
    } else {
        thisProxy[parent].Receive(fragment, size, edges, weights, author);
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

void MST::Answer()
{
    int buf = true;
    const CkCallback cb(CkReductionTarget(Main, reduce), mainProxy);
    contribute(sizeof(int), &buf, CkReduction::max_int, cb);
}

#include "mst.def.h"
