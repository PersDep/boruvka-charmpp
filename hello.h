#ifndef __HELLO_H__
#define __HELLO_H__

#include <stdexcept>
#include <vector>

#include "deferrment.h"

using namespace std;

typedef vector<vector<edge_id_t > > result_t;

class Hello : public CBase_Hello {
	Hello_SDAG_CODE

 private:
  Graph graph;
  int parent;
  int active;

 public:

  /// Constructors ///
  Hello(int nVertices, int nEdges, int *edges, double *weights);
  Hello(CkMigrateMessage *msg);

  /// Entry Methods ///
  void ProcessFragment(int root);
  void Receive(map<int, bool> fragment);
  void UpdateParent(int child, int parent);
  void PromoteRank(int parent);

};


#endif //__HELLO_H__
