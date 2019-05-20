#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <assert.h>
#include <math.h>

class Main : public CBase_Main {

 public:

  /// Constructors ///
  Main(CkArgMsg* msg);
  Main(CkMigrateMessage* msg);

  /// Entry Methods ///
  void done();

  void usage(int argc, char **argv);
  void init(int argc, char** argv, graph_t* G);
  void readGraph(graph_t *G, char *filename);
  void freeGraph(graph_t *G);

};


#endif //__MAIN_H__
