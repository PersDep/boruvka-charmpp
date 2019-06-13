#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <assert.h>
#include <math.h>

#include "deferrment.h"

typedef vector<vector<edge_id_t > > result_t;

class Main : public CBase_Main {

private:
  graph_t g;
  Graph graph;
  result_t mst;
  int nTrees, mstCounter;
  struct timespec start_ts, finish_ts;

  CProxy_Hello helloArray;
  

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

  void MST(graph_t *G);
  void ContinueMST();
  void convert_to_output(graph_t *G, void* result, forest_t *trees_output);
  void write_output_information(forest_t *trees, char *filename);
  // void reduce(CkReductionMsg *msg);
  void reduce(int uniteAmount);
  void push(int id);
  void noConnections();
};


#endif //__MAIN_H__
