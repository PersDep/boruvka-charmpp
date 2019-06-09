#include <algorithm>
#include <vector>
#include <cassert>

#include <error.h>

#include "deferrment.h"

#include "hello.decl.h"

#include "hello.h"
#include "main.decl.h"

using namespace std;

char inFilename2[FNAME_LEN];
char outFilename2[FNAME_LEN];

extern /* readonly */ CProxy_Main mainProxy;


Hello::Hello() {
  CkPrintf("Started running MST\n");
  // Nothing to do when the Hello chare object is created.
  //   This is where member variables would be initialized
  //   just like in a C++ class constructor.
  this->G = new graph_t;
  char **args = new char*[3];
  args[1] = "-in";
  args[2] = "rmat-10";
  CkPrintf("Going to init\n");
  init(3, args, G); 
  CkPrintf("Inited\n");
  readGraph(G, inFilename2);
  CkPrintf("Read graph\n");
  this->trees_output = new forest_t;
  CkPrintf("MST constructor finished\n");
}


// Constructor needed for chare object migration (ignore for now)
// NOTE: This constructor does not need to appear in the ".ci" file
Hello::Hello(CkMigrateMessage *msg) { }

void Hello ::MST( void ) {
    Graph graph(G->n, G->m, G);
    double mstWeight = 0;
    int nTrees = graph.nVertices, mstCounter = 0;
    mst.clear();
    mst.push_back(vector<edge_id_t>());
    while (nTrees > 1) {
        graph.InitCheapestEdges();
        for (int i = 0; i < graph.nEdges; i++)
            graph.CheckEdge(graph.Find(graph.edges[i].src), graph.Find(graph.edges[i].dest), i);
        bool cheapestExists = false;
        for (int i = 0; i < graph.nVertices; i++)
            if (graph.cheapestEdges[i] != -1) {
                cheapestExists = true;
                int set1 = graph.Find(graph.edges[graph.cheapestEdges[i]].src);
                int set2 = graph.Find(graph.edges[graph.cheapestEdges[i]].dest);
                if (set1 == set2)
                  continue;
                mstWeight += graph.edges[graph.cheapestEdges[i]].weight;
                mst[mstCounter].push_back(graph.edges[graph.cheapestEdges[i]].id);
                graph.Unite(set1, set2);
                nTrees--;
            }
        if (!cheapestExists) {
            mst.push_back(vector<edge_id_t>());
            mstCounter++;
            nTrees--;
        }
    }
}

void Hello ::convert_to_output( void )
{
    result_t &trees_mst = *reinterpret_cast<result_t*>(&mst);
    trees_output->p_edge_list = (edge_id_t *)malloc(trees_mst.size()*2 * sizeof(edge_id_t));
    edge_id_t number_of_edges = 0;
    for (vertex_id_t i = 0; i < trees_mst.size(); i++) number_of_edges += trees_mst[i].size();
    trees_output->edge_id = (edge_id_t *)malloc(number_of_edges * sizeof(edge_id_t));
    trees_output->p_edge_list[0] = 0;
    trees_output->p_edge_list[1] = trees_mst[0].size();
    for (vertex_id_t i = 1; i < trees_mst.size(); i++) {
      trees_output->p_edge_list[2*i] = trees_output->p_edge_list[2*i-1];
      trees_output->p_edge_list[2*i +1] = trees_output->p_edge_list[2*i-1] + trees_mst[i].size();
    }
    int k = 0;
    for (vertex_id_t i = 0; i < trees_mst.size(); i++)
      for (edge_id_t j = 0; j < trees_mst[i].size(); j++) {
        trees_output->edge_id[k] = trees_mst[i][j];
        k++;
      }
    trees_output->numTrees = trees_mst.size();
    trees_output->numEdges = number_of_edges;

    write_output_information(trees_output, outFilename2);
    free(trees_output->p_edge_list);
    free(trees_output->edge_id);

    printf("trees = %u\n", trees_output->numTrees);

    free(trees_output);
    freeGraph(G);
    free(G);
    mainProxy.done();
}

void Hello::write_output_information(forest_t *trees, const char *filename)
{
    FILE *F = fopen(filename, "wb");
    assert(fwrite(&trees->numTrees, sizeof(vertex_id_t), 1, F) == 1);
    assert(fwrite(&trees->numEdges, sizeof(edge_id_t), 1, F) == 1);
    assert(fwrite(trees->p_edge_list, sizeof(edge_id_t), 2*trees->numTrees, F) == 2*trees->numTrees);
    assert(fwrite(trees->edge_id, sizeof(edge_id_t), trees->numEdges, F) == trees->numEdges);
    fclose(F);
}

void Hello::usage(int argc, char **argv)
{
    printf("Usage:\n");
    printf("    %s -in <input> [options] \n", argv[0]);
    printf("Options:\n");
    printf("    -in <input> -- input graph filename\n");
    printf("    -out <output> -- algorithm result will be placed to output filename. It can be used in validation. <in>.mst is default value.\n");
    printf("    -nIters <nIters> -- number of iterations. By default 64\n");
    exit(1);
}

void Hello::init(int argc, char** argv, graph_t* G)
{
    int l, buf;
    inFilename2[0] = '\0';
    outFilename2[0] = '\0';
    bool no_in_filename = true; 

    if (argc == 1) usage(argc, argv);

    CkPrintf("Initing\n");

    for (int i = 1; i < argc; ++i) {
   		if (!strcmp(argv[i], "-in")) {
            l = strlen(argv[++i]);
            strncpy(inFilename2, argv[i], (l > FNAME_LEN-1 ? FNAME_LEN-1 : l) );
            no_in_filename = false;
            CkPrintf("filename: %s\n", inFilename2);
        }
   		if (!strcmp(argv[i], "-out")) {
            l = strlen(argv[++i]);
            strncpy(outFilename2, argv[i], (l > FNAME_LEN-1 ? FNAME_LEN-1 : l) );
        }
		if (!strcmp(argv[i], "-nIters")) {
			buf = (int) atoi(argv[++i]);
        }
    }
    if (no_in_filename) usage(argc, argv);
    if (strlen(outFilename2) == 0) sprintf(outFilename2, "%s.mst", inFilename2);
}

void Hello::readGraph(graph_t *G, char *filename)
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

void Hello::freeGraph(graph_t *G)
{
    free(G->rowsIndices);
    free(G->endV);
    free(G->weights);
}


#include "hello.def.h"
