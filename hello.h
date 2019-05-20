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
	graph_t *G;
  result_t mst;
  forest_t *trees_output;
  
 public:

  /// Constructors ///
  Hello();
  Hello(CkMigrateMessage *msg);

  /// Entry Methods ///
  //void receive(int from, int sender_phase, int direction);
  void convert_to_output();
  // void receive_impl(int from, int sender_phase, int direction);
  //void receive_process( void );
  void MST( void );
  void write_output_information(forest_t *trees, const char *filename);
  void usage(int argc, char **argv);
  void init(int argc, char** argv, graph_t* G);
  void readGraph(graph_t *G, char *filename);
  void freeGraph(graph_t *G);

};


#endif //__HELLO_H__
