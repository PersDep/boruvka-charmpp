#include "hello.decl.h"

#include "hello.h"
#include "main.decl.h"

#include <cassert>

#include "deferrment.h"

extern /* readonly */ CProxy_Main mainProxy;


Hello::Hello() {

  CkPrintf("Started running Hello %d %d\n", 5, 10);
  // Nothing to do when the Hello chare object is created.
  //   This is where member variables would be initialized
  //   just like in a C++ class constructor.
  this->size = 5;
  this->p = 10;
  this->current_p = -1;
  this->neighbors = 2;
  this->n_received = 0;

  //Signal my neighbors
  this->left_neighbor = (thisIndex+this->size-1)%(this->size);
  this->right_neighbor = (thisIndex+1)%(this->size);


  //this->the_msg_queue();
  this->my_queues.expect(this->left_neighbor);
  this->my_queues.expect(this->right_neighbor);
}


// Constructor needed for chare object migration (ignore for now)
// NOTE: This constructor does not need to appear in the ".ci" file
Hello::Hello(CkMigrateMessage *msg) { }


void Hello ::receive(int from, int sender_phase, int direction) {
  // Have this chare object say hello to the user.
  CkPrintf("Element  %d (phase %d) on processor %d RECEIVED from %d (p=%d) in direction %d.\n",
           thisIndex, this->current_p, CkMyPe(), from, sender_phase, direction);

  this->my_queues.add(from,sender_phase,direction);

	if( this->my_queues.all_ready() ){
		//CkPrintf("Element  %d (phase %d) on processor %d all are ready.\n",
		//         thisIndex, this->current_p, CkMyPe());

		MultisenderOrderedMessageQueue<int>::iterator itr;
		//itr = this->my_queues.begin();
		//std::cout << " iterator value is " << *itr << " blah " << std::endl;

		for(itr = this->my_queues.begin(); itr != this->my_queues.end(); ++itr){
			//CkPrintf("Element  %d (phase %d) on processor %d : examine queue %d.\n",
			//         thisIndex, this->current_p, CkMyPe(), *itr);
			MultisenderOrderedMessageQueue<int>::msg_t the_msg = this->my_queues.pop(*itr);
			this->receive_impl(the_msg.origin, the_msg.number, the_msg.value);
		}

		this->new_phase();//not called as an entry method.
		//this->receive_impl(one_message.first, one_message.second.first, one_message.second.second);
	}
}

void Hello ::receive_impl(int from, int sender_phase, int direction) {

  // Have this chare object say hello to the user.
  CkPrintf("Element  %d (phase %d) on processor %d PROCESSED from %d (p=%d) in direction %d.\n",
           thisIndex, this->current_p, CkMyPe(), from, sender_phase, direction);

  CkAssert(sender_phase == this->current_p);
  assert(sender_phase == this->current_p);

  this->n_received++;
}

void Hello ::new_phase( void ) {

  if(this->current_p > this->p){
	  //done
	  mainProxy.done();
	  return;
  }

  this->current_p++;
  // Have this chare object say hello to the user.
  CkPrintf("Element  %d on processor %d starting phase %d, has received %d.\n",
           thisIndex, CkMyPe(), this->current_p, this->n_received);


   this->n_received = 0;
  //Signal my neighbors
  	thisProxy[this->left_neighbor].receive(thisIndex, this->current_p, -1);
  	thisProxy[this->right_neighbor].receive(thisIndex, this->current_p, 1);

}


#include "hello.def.h"
