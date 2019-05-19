#ifndef __HELLO_H__
#define __HELLO_H__

#include <stdexcept>

#include "deferrment.h"

class Hello : public CBase_Hello {
	Hello_SDAG_CODE
private:
	int size;
	int p;
	int current_p;
	int neighbors;
	int n_received;

	int left_neighbor;
	int right_neighbor;

	MultisenderOrderedMessageQueue<int> my_queues;
 public:

  /// Constructors ///
  Hello();
  Hello(CkMigrateMessage *msg);

  /// Entry Methods ///
  //void receive(int from, int sender_phase, int direction);
  void receive(int from, int sender_phase, int direction);
  void receive_impl(int from, int sender_phase, int direction);
  //void receive_process( void );
  void new_phase( void );

};


#endif //__HELLO_H__
