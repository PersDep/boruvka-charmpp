#ifndef _GRAPH_HPP_
#define _GRAPH_HPP_

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <vector>

#define DEFAULT_ARITY 16
#define SMALL_COMPONENT_EDGES_THRESHOLD   2
#define FNAME_LEN   512
#define WEIGHT_ERROR 0.0001

typedef uint32_t vertex_id_t;
typedef uint64_t edge_id_t;
typedef double weight_t;

using namespace std;

/* The graph data structure*/
typedef struct
{
    /***
     The minimal graph repesentation consists of:
     n        -- the number of vertices
     m        -- the number of edges
     endV     -- an array of size m that stores the 
                 destination ID of an edge <src->dest>.
     rowsIndices -- an array of size n+1 that stores the degree 
                 (out-degree in case of directed graphs) and pointers to
                 the endV array. The degree of vertex i is given by 
                 rowsIndices[i+1]-rowsIndices[i], and the edges out of i are
                 stored in the contiguous block endV[rowsIndices[i] .. rowsIndices[i+1]-1].
     Vertices are ordered from 0 in our internal representation
     ***/
    vertex_id_t n;
    edge_id_t m;
    edge_id_t* rowsIndices;
    vertex_id_t* endV;

    /* Edge weights */
    weight_t* weights;
    weight_t min_weight, max_weight;

    /* other graph parameters */
    int scale; /* log2 of vertices number */
    int avg_vertex_degree; /* relation m / n */
    bool directed; 

    /* RMAT graph parameters */
    double a, b, c;     
    bool permute_vertices;
    
    /* Distributed version variables */
    int nproc, rank;
    vertex_id_t local_n; /* local vertices number */
    edge_id_t local_m; /* local edges number */
    edge_id_t* num_edges_of_any_process;

    char filename[FNAME_LEN]; /* filename for output graph */
} graph_t;

typedef struct
{
    vertex_id_t numTrees;
    edge_id_t numEdges;
    edge_id_t* p_edge_list;
    edge_id_t* edge_id;

} forest_t;

struct Edge
{
	double weight;
	int id, src, dest;

	Edge(): id(-1), src(-1), dest(-1), weight(-1) {}
	Edge(int id, int src, int dest, double weight): id(id), src(src), dest(dest), weight(weight) {}
};

struct EmbeddedEdge
{
	int id, src, dest;

	EmbeddedEdge(): id(-1), src(-1), dest(-1) {}
	EmbeddedEdge(int id, int src, int dest): id(id), src(src), dest(dest) {}
	EmbeddedEdge(const Edge &edge): id(edge.id), src(edge.src), dest(edge.dest) {}
};

struct Subset
{
	int parent, rank;

	Subset(): parent(-1), rank(-1) {}
	Subset(int parent, int rank = 0): parent(parent), rank(rank) {}
};

struct Graph
{
    int nVertices, nEdges;
    vector<Edge> edges;
    vector<Subset> subsets;
    map<int, map<int, bool>> fragments;
	vector<int> cheapestEdges;

	Graph(): nVertices(0), nEdges(0) { }

    Graph(int nVertices, int nEdges, graph_t *rmatGraph = nullptr, bool meta = true): nVertices(nVertices), nEdges(nEdges)
    {
    	edges = vector<Edge>(nEdges);
    	if (rmatGraph)
	        for (vertex_id_t i = 0; i < rmatGraph->n; i++) {
				if (meta) {
					subsets.push_back(Subset(i));
					fragments[i][i] = true;
				}
		        for (edge_id_t j = rmatGraph->rowsIndices[i]; j < rmatGraph->rowsIndices[i + 1]; j++)
			        edges[j] = Edge(j, i, rmatGraph->endV[j], rmatGraph->weights[j]);
	        }
    }

	Graph(int nVertices, int nEdges, int parent, EmbeddedEdge *embeddedEdges, double *weights): nVertices(nVertices), nEdges(nEdges)
	{
		for (int i = 0; i < nEdges; i++)
            edges.push_back(Edge(embeddedEdges[i].id, embeddedEdges[i].src, embeddedEdges[i].dest, weights[i]));
		for (int i = 0; i < nVertices; i++)
			subsets.push_back(Subset(i));
		fragments[parent][parent] = true;
	}

    Graph(int nVertices, int nEdges, int parent): nVertices(nVertices), nEdges(nEdges)
	{
		for (int i = 0; i < nVertices; i++)
			subsets.push_back(Subset(i));
		fragments[parent][parent] = true;
	}

    void InitCheapestEdges() { cheapestEdges = vector<int>(nVertices, -1); }

	int Find(int i) { return subsets[i].parent; }

    void CheckEdge(int set1, int set2, int i)
    {
        if (set1 == set2) {
	        return;
        }
	    if (cheapestEdges[set1] == -1 || edges[cheapestEdges[set1]].weight > edges[i].weight)
		    cheapestEdges[set1] = i;
    }

    void Unite(int xroot, int yroot)
    {
	    if (subsets[xroot].rank < subsets[yroot].rank) {
		    subsets[xroot].parent = yroot;
		    subsets[yroot].rank++;
		    UpdateFragments(xroot, yroot);
	    } else {
		    subsets[yroot].parent = xroot;
		    subsets[xroot].rank++;
		    UpdateFragments(yroot, xroot);
	    }
    }

    void UpdateFragments(int oldRoot, int newRoot)
    {
	    fragments[newRoot].insert(fragments[oldRoot].begin(), fragments[oldRoot].end());
	    fragments.erase(oldRoot);
	    for (auto &child : fragments[newRoot])
	        subsets[child.first].parent = newRoot;
    }

    void PrintFragments()
    {
    	for (auto &fragment : fragments) {
		    cout << fragment.first << ": ";
		    for (auto &child : fragment.second)
			    cout << child.first << " ";
		    cout << endl;
	    }
    }
};


#endif /* _GRAPH_HPP_ */
