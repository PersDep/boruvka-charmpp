#ifndef _DEFERRMENT_HPP_
#define _DEFERRMENT_HPP_

//#define DEBUG 1
#ifdef DEBUG
#include <iostream>
#endif

#include <cassert>
#include <set>
#include <unordered_map>
#include <queue>

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
