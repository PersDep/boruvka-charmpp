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
  int working;

 public:

  /// Constructors ///
  Hello();
  Hello(CkMigrateMessage *msg);

  /// Entry Methods ///
  void ProcessFragment(int, int, int, int, int *, int, double *, int , int *, int, int *);
  void Receive(map<int, bool> fragment, int id);
  void UpdateParent(int child, int parent);
  void PromoteRank(int parent);

};


#endif //__HELLO_H__
