#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <float.h>

#include "deferrment.h"

#include "main.decl.h"

#include "main.h"
#include "hello.decl.h"

char inFilename[FNAME_LEN];
char outFilename[FNAME_LEN];
int nIters = 64;

#if defined(CLOCK_MONOTONIC)
#define CLOCK CLOCK_MONOTONIC
#elif defined(CLOCK_REALTIME)
#define CLOCK CLOCK_REALTIME
#else
#error "Failed to find a timing clock."
#endif

/* readonly */ CProxy_Main mainProxy;


// Entry point of Charm++ application
Main::Main(CkArgMsg* msg) {
    CkPrintf("Running MST using %d processors.\n", CkNumPes());

  // Set the mainProxy readonly to point to a
  //   proxy for the Main chare object (this
  //   chare object).
    mainProxy = thisProxy;

  // Invoke the "sayHi()" entry method on all of the
  //   elements in the helloArray array of chare objects.
  // helloArray.new_phase();
  //helloArray.wrong_order_presend_start();

    graph_t g;
    struct timespec start_ts, finish_ts;
    double *perf;
    // forest_t trees_output;
    /* initializing and reading the graph */
    init(msg->argc, msg->argv, &g); 
    readGraph(&g, inFilename);
    // init_mst(&g);

      // Create the array of Hello chare objects.
    CProxy_Hello helloArray = CProxy_Hello::ckNew(1);

    perf = (double *)malloc(nIters * sizeof(double));
    // void *result = 0;

    printf("start algorithm iterations...\n");
    for (int i = 0; i < nIters; ++i) {
        printf("\tMST %d\t ...",i); fflush(NULL);
        clock_gettime(CLOCK, &start_ts);
        helloArray.MST();
        clock_gettime(CLOCK, &finish_ts);
        double time = (finish_ts.tv_nsec - (double)start_ts.tv_nsec) * 1.0e-9 + (finish_ts.tv_sec - (double)start_ts.tv_sec);
        perf[i] = g.m / (1000000 * time);
        printf("\tfinished. Time is %.4f secs\n", time);
    }
    printf("algorithm iterations finished.\n");

    helloArray.convert_to_output();
    // write_output_information(&trees_output, outFilename);
    // free(trees_output.p_edge_list);
    // free(trees_output.edge_id);

    /* final print */
    double min_perf, max_perf, avg_perf;
    max_perf = avg_perf = 0;
    min_perf = DBL_MAX;
    for (int i = 0; i < nIters; ++i) {
    	avg_perf += perf[i];
        if (perf[i] < min_perf) min_perf = perf[i];
        if (perf[i] > max_perf) max_perf = perf[i];
    }
    avg_perf /= nIters;

    printf("%s: vertices = %d edges = %lld nIters = %d MST performance min = %.4f avg = %.4f max = %.4f MTEPS\n", 
            inFilename, g.n, (long long)g.m, nIters, min_perf, avg_perf, max_perf);
    printf("Performance = %.4f MTEPS\n", avg_perf);
    free(perf);
    freeGraph(&g);
    // finalize_mst(&g);
    delete msg;
}


// Constructor needed for chare object migration (ignore for now)
// NOTE: This constructor does not need to appear in the ".ci" file
Main::Main(CkMigrateMessage* msg) { }

void Main::usage(int argc, char **argv)
{
    printf("Usage:\n");
    printf("    %s -in <input> [options] \n", argv[0]);
    printf("Options:\n");
    printf("    -in <input> -- input graph filename\n");
    printf("    -out <output> -- algorithm result will be placed to output filename. It can be used in validation. <in>.mst is default value.\n");
    printf("    -nIters <nIters> -- number of iterations. By default 64\n");
    exit(1);
}

void Main::init(int argc, char** argv, graph_t* G)
{
    int l;
    inFilename[0] = '\0';
    outFilename[0] = '\0';
    bool no_in_filename = true; 

    if (argc == 1) usage(argc, argv);

    for (int i = 1; i < argc; ++i) {
   		if (!strcmp(argv[i], "-in")) {
            l = strlen(argv[++i]);
            strncpy(inFilename, argv[i], (l > FNAME_LEN-1 ? FNAME_LEN-1 : l) );
            no_in_filename = false;
        }
   		if (!strcmp(argv[i], "-out")) {
            l = strlen(argv[++i]);
            strncpy(outFilename, argv[i], (l > FNAME_LEN-1 ? FNAME_LEN-1 : l) );
        }
		if (!strcmp(argv[i], "-nIters")) {
			nIters = (int) atoi(argv[++i]);
        }
    }
    if (no_in_filename) usage(argc, argv);
    if (strlen(outFilename) == 0) sprintf(outFilename, "%s.mst", inFilename);
}

void Main::readGraph(graph_t *G, char *filename)
{
    uint8_t align;
    FILE *F = fopen(filename, "rb");
    if (!F) error(EXIT_FAILURE, 0, "Error in opening file %s", filename);
    
    assert(fread(&G->n, sizeof(vertex_id_t), 1, F) == 1);
    G->scale = log(G->n) / log (2);
    
    assert(fread(&G->m, sizeof(edge_id_t), 1, F) == 1);
    assert(fread(&G->directed, sizeof(bool), 1, F) == 1);
    assert(fread(&align, sizeof(uint8_t), 1, F) == 1);
    
    G->rowsIndices = (edge_id_t *)malloc((G->n+1) * sizeof(edge_id_t));
    assert(G->rowsIndices); 
    assert(fread(G->rowsIndices, sizeof(edge_id_t), G->n+1, F) == (G->n+1));
    G->endV = (vertex_id_t *)malloc(G->rowsIndices[G->n] * sizeof(vertex_id_t));
    assert(G->endV);
    assert(fread(G->endV, sizeof(vertex_id_t), G->rowsIndices[G->n], F) == G->rowsIndices[G->n]);
    G->weights = (weight_t *)malloc(G->m * sizeof(weight_t));
    assert(G->weights);

    assert(fread(G->weights, sizeof(weight_t), G->m, F) == G->m);
    fclose(F);
}

void Main::freeGraph(graph_t *G)
{
    free(G->rowsIndices);
    free(G->endV);
    free(G->weights);
}


// When called, the "done()" entry method will cause the program
//   to exit.
void Main::done() {
    CkExit();
}


#include "main.def.h"
