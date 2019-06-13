#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <assert.h>
#include <math.h>

#include "graph.h"

typedef vector<vector<edge_id_t > > result_t;

class Main : public CBase_Main {

private:
  graph_t g;
  Graph graph;
  result_t mst;
  int nTrees, mstCounter;
  struct timespec start_ts, finish_ts;

  CProxy_MST mstArray;
  

 public:

  /// Constructors ///
  Main(CkArgMsg* msg);
  Main(CkMigrateMessage* msg);

  /// Entry Methods ///
  void MST(graph_t *G);
  void reduce(int uniteAmount);
  void push(int id);
  void noConnections();
  void init(int argc, char** argv, graph_t* G);
  void usage(int argc, char **argv);
  void readGraph(graph_t *G, char *filename);
  void freeGraph(graph_t *G);
  void convert_to_output(graph_t *G, void* result, forest_t *trees_output);
  void write_output_information(forest_t *trees, char *filename);
  void done();
};


#endif //__MAIN_H__
