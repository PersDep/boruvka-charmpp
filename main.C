#include "main.decl.h"

#include "main.h"
#include "hello.decl.h"


/* readonly */ CProxy_Main mainProxy;


// Entry point of Charm++ application
Main::Main(CkArgMsg* msg) {

  // Initialize the local member variables
  doneCount = 0;    // Set doneCount to 0
  numElements = 5;  // Default numElements to 5

  // There should be 0 or 1 command line arguements.
  //   If there is one, it is the number of "Hello"
  //   chares that should be created.
  if (msg->argc > 1)
    numElements = atoi(msg->argv[1]);

  // We are done with msg so delete it.
  delete msg;

  // Display some info about this execution
  //   for the user.
  CkPrintf("Running \"Hello World\" with %d elements "
           "using %d processors.\n",
           numElements, CkNumPes());

  // Set the mainProxy readonly to point to a
  //   proxy for the Main chare object (this
  //   chare object).
  mainProxy = thisProxy;

  // Create the array of Hello chare objects.
  CProxy_Hello helloArray = CProxy_Hello::ckNew(numElements);

  // Invoke the "sayHi()" entry method on all of the
  //   elements in the helloArray array of chare objects.
  helloArray.new_phase();
  //helloArray.wrong_order_presend_start();
}


// Constructor needed for chare object migration (ignore for now)
// NOTE: This constructor does not need to appear in the ".ci" file
Main::Main(CkMigrateMessage* msg) { }


// When called, the "done()" entry method will cause the program
//   to exit.
void Main::done() {

  // Increment the doneCount.  If all of the Hello chare
  //   objects have indicated that they are done, then exit.
  //   Otherwise, continue waiting for the Hello chare objects.
  doneCount++;
  if (doneCount >= numElements)
    CkExit();
}


#include "main.def.h"
