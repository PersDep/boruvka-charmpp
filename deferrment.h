#ifndef _DEFERRMENT_HPP_
#define _DEFERRMENT_HPP_

#include <algorithm>
#include <cassert>
#include <iostream>
#include <set>
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>

#define DEFAULT_ARITY 16
#define SMALL_COMPONENT_EDGES_THRESHOLD   2
#define FNAME_LEN   256
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

struct Graph
{
    struct Edge
    {
		int id, src, dest;
		double weight;

		Edge(): id(-1), src(-1), dest(-1), weight(-1) {}
		Edge(int id, int src, int dest, double weight): id(id), src(src), dest(dest), weight(weight) {}
    };

    struct Subset
    {
		int parent, rank;

		Subset(): parent(-1), rank(-1) {}
		Subset(int parent, int rank = 0): parent(parent), rank(rank) {}
    };


    int nVertices, nEdges;
    vector<Edge> edges;
    vector<Subset> subsets;
    vector<int> cheapestEdges;
    map<int, map<int, bool>> fragments;

    Graph(int nVertices, int nEdges, graph_t *rmatGraph = nullptr): nVertices(nVertices), nEdges(nEdges)
    {
    	edges = vector<Edge>(nEdges);
    	subsets = vector<Subset>(nVertices);
    	if (rmatGraph)
	        for (vertex_id_t i = 0; i < rmatGraph->n; i++) {
		        subsets[i] = Subset(i);
		        fragments[i][i] = true;
		        for (edge_id_t j = rmatGraph->rowsIndices[i]; j < rmatGraph->rowsIndices[i + 1]; j++)
			        edges[j] = Edge(j, i, rmatGraph->endV[j], rmatGraph->weights[j]);
	        }
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
		    UpdateFragments(xroot, yroot);
	    } else if (subsets[xroot].rank > subsets[yroot].rank) {
		    subsets[yroot].parent = xroot;
		    UpdateFragments(yroot, xroot);
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

template <class T>
class OrderedMessageQueue {
	private:
		int msg_number = 0;
		std::priority_queue< std::pair<int, T>,
				std::vector<std::pair<int, T>>,
				std::greater<std::pair<int, T>>
				> my_queue;
	public:
		OrderedMessageQueue(){};
		bool add(int number, T obj);
		bool ready() const;
		int size() const;
		//int current_phase() const;
		typedef struct { int number; T value; } msg_t;
		msg_t pop();
};

template <class T>
class MultisenderOrderedMessageQueue {
	private:
		std::unordered_map<int, OrderedMessageQueue<T> > my_map;
		std::set<int> my_keyset;
	public:

		MultisenderOrderedMessageQueue(){};
		void expect(int origin);
		bool add(int origin, int number, T obj);
		bool ready() const;//Any
		bool ready(int origin) const;
		bool all_ready() const;
		//bool all_ready(int phase) const;
		typedef struct { int origin; int number; T value; } msg_t;
		//int size();
		msg_t pop();//Any that is ready
		msg_t pop(int origin);

		typedef typename std::set<int>::iterator iterator;
		typedef typename std::set<int>::const_iterator const_iterator;

		iterator begin(){return this->my_keyset.begin();}
		iterator end(){return this->my_keyset.end();}
};



template <class T>
bool OrderedMessageQueue<T>::add(int number, T obj){
	this->my_queue.push(std::make_pair(number, obj));
	return this->ready();
}

template <class T>
bool OrderedMessageQueue<T>::ready() const{
	if(this->my_queue.size() == 0) return false;
	return this->my_queue.top().first == this->msg_number;
}

template <class T>
int OrderedMessageQueue<T>::size() const{
	return this->my_queue.size();
}

template <class T>
typename OrderedMessageQueue<T>::msg_t OrderedMessageQueue<T>::pop(){
	if(! this->ready()) throw std::runtime_error("Message queue not ready");


	std::pair<int, T> to_return_tmp =  this->my_queue.top();
	this->my_queue.pop();
	this->msg_number++;

	OrderedMessageQueue<T>::msg_t to_return;
	to_return.number = to_return_tmp.first;
	to_return.value = to_return_tmp.second;
	return to_return;
}

template <class T>
void MultisenderOrderedMessageQueue<T>::expect(int origin){
	//this->my_queue.push(std::make_pair(number, obj));
	this->my_keyset.insert(origin);
	int this_goes_unused = this->my_map[origin].size();
}


template <class T>
bool MultisenderOrderedMessageQueue<T>::add(int origin, int number, T obj){
	//this->my_queue.push(std::make_pair(number, obj));
	this->my_keyset.insert(origin);
	return this->my_map[origin].add(number, obj);
}

template <class T>
bool MultisenderOrderedMessageQueue<T>::ready() const{
	//TODO: Keep track of being ready another way.

	//if(this->my_queue.size() == 0) return false;
	//return this->my_queue.top().first == this->msg_number;
	typename std::unordered_map<int, OrderedMessageQueue<T> >::const_iterator itr ;
	for(itr = this->my_map.begin();itr!=this->my_map.end();++itr){
		if(itr->second.ready()) return true;
	}
	return false;
}

template <class T>
bool MultisenderOrderedMessageQueue<T>::ready(int origin) const{
	//if(this->my_queue.size() == 0) return false;
	//return this->my_queue.top().first == this->msg_number;
	return this->my_map[origin].ready();
}

template <class T>
bool MultisenderOrderedMessageQueue<T>::all_ready() const{
	bool all_are_ready = true;
	//TODO: Keep track of being ready another way.

	//if(this->my_queue.size() == 0) return false;
	//return this->my_queue.top().first == this->msg_number;
	typename std::unordered_map<int, OrderedMessageQueue<T> >::const_iterator itr ;
	for(itr = this->my_map.begin();itr!=this->my_map.end();++itr){
		bool one_ready = itr->second.ready();
		all_are_ready = all_are_ready && one_ready;
		//are_all_ready = are_all_ready && itr->second.ready();
		#ifdef DEBUG
			std::cerr << "Checking to see if ready " << one_ready << std::endl;
		#endif


	}
	#ifdef DEBUG
		std::cerr << "All ready? " << all_are_ready << std::endl;
	#endif
	return all_are_ready;
}

template <class T>
typename MultisenderOrderedMessageQueue<T>::msg_t MultisenderOrderedMessageQueue<T>::pop(){
	MultisenderOrderedMessageQueue<T>::msg_t msg_to_return;

	typename std::unordered_map<int, OrderedMessageQueue<T> >::iterator itr ;
	for(itr = this->my_map.begin();itr!=this->my_map.end();++itr){
		if(itr->second.ready()){

			typename OrderedMessageQueue<T>::msg_t tmp_submessage = itr->second.pop();

			msg_to_return.origin = itr->first;
			msg_to_return.number = tmp_submessage.number;
			msg_to_return.value = tmp_submessage.value;

			return msg_to_return;
		}
	}

	throw std::runtime_error("Tried to pop MultiMessage but no queue ready");
}

template <class T>
typename MultisenderOrderedMessageQueue<T>::msg_t MultisenderOrderedMessageQueue<T>::pop(int origin){
	typename MultisenderOrderedMessageQueue<T>::msg_t msg_to_return;
	typename OrderedMessageQueue<T>::msg_t tmp_submessage = this->my_map[origin].pop();

	msg_to_return.origin = origin;
	msg_to_return.number = tmp_submessage.number;
	msg_to_return.value = tmp_submessage.value;

	return msg_to_return;
}


#endif /* _DEFERRMENT_HPP_ */
