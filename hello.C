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


Hello::Hello() {
  CkPrintf("Started running ProcessFragment on chare %d\n", thisIndex);
  // Nothing to do when the Hello chare object is created.
  //   This is where member variables would be initialized
  //   just like in a C++ class constructor.

}

// Constructor needed for chare object migration (ignore for now)
// NOTE: This constructor does not need to appear in the ".ci" file
Hello::Hello(CkMigrateMessage *msg) { }


void Hello::ProcessFragment(Graph graph)
// void Hello::ProcessFragment()
{
    CkPrintf("Running ProcessFragment\n");
    CkPrintf("Running one of %d fragments", graph.fragments.size());
    size_t fragmentNum = thisIndex;
    vector<UniteInfo> pairs;
    for (int i = 0; i < graph.nEdges; i++)
        if (graph.fragments[fragmentNum].count(graph.edges[i].src))
      	    graph.CheckEdge(graph.Find(graph.edges[i].src), graph.Find(graph.edges[i].dest), i);
    for (auto &child : graph.fragments[fragmentNum]) {
        int i = child.first;
      	if (graph.cheapestEdges[i] != -1) {
      		int set1 = graph.Find(graph.edges[graph.cheapestEdges[i]].src);
      		int set2 = graph.Find(graph.edges[graph.cheapestEdges[i]].dest);
      		if (set1 != set2) {
            UniteInfo uniteInfo;
            uniteInfo.set1 = set1;
            uniteInfo.set2 = set2;
            uniteInfo.edgeId = graph.edges[graph.cheapestEdges[i]].id;
       		 pairs.push_back(uniteInfo);
          }
        }
    }
    // UniteInfo uniteInfo;
    // uniteInfo.set1 = 5;
    // uniteInfo.set2 = 7;
    // uniteInfo.edgeId = 2;
    // pairs.push_back(uniteInfo);
    //pairs reduction
    const CkCallback cb(CkReductionTarget(Main, reduce), mainProxy);
    CkPrintf("Time to contribute\n");
    contribute(pairs, CkReduction::concat, cb);
    mainProxy.done();
}


#include "hello.def.h"
