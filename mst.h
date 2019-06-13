#ifndef __MST_H__
#define __MST_H__

#include <stdexcept>
#include <vector>

#include "graph.h"

using namespace std;

typedef vector<vector<edge_id_t > > result_t;

class MST : public CBase_MST {
	MST_SDAG_CODE

 private:
  Graph graph;
  int parent;
  int active;

 public:

  /// Constructors ///
  MST(int nVertices, int nEdges, int *edges, double *weights);
  MST(CkMigrateMessage *msg);

  /// Entry Methods ///
  void ProcessFragment(int root);
  void Receive(map<int, bool> fragment);
  void UpdateParent(int child, int parent);
  void PromoteRank(int parent);

};


#endif //__MST_H__
