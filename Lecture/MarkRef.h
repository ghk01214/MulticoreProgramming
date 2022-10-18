#pragma once

#include "pch.h"
#include "Node.h"

template<typename T>
class Node;

template<typename T>
class MarkRef
{
public:
	MarkRef();
	MarkRef(Node<T>* addr, bool mark);

	Node<T>* get();
	void reset(Node<T>* ptr);
	bool removed();
	bool try_removed(Node<T>* node, bool removed);

	Node<T>* get_n_mark(bool* removed);

	bool cas(Node<T>* old_ptr, bool old_mark, Node<T>* new_ptr, bool new_mark);

private:
	uint64_t address;
};

template<typename T>
inline MarkRef<T>::MarkRef() :
	address{ 0 }
{
}

template<typename T>
inline MarkRef<T>::MarkRef(Node<T>* addr, bool mark)
{
	address = reinterpret_cast<uint64_t>(addr);

	if (mark == true)
		address = address | 1;
}

template<typename T>
inline Node<T>* MarkRef<T>::get()
{
	return reinterpret_cast<Node<T>*>(address & 0xFFFFFFFFFFFFFFFE);
}

template<typename T>
inline void MarkRef<T>::reset(Node<T>* ptr)
{
	address = reinterpret_cast<uint64_t>(ptr);
}

template<typename T>
inline bool MarkRef<T>::removed()
{
	return (address & 1) == 1;
}

template<typename T>
inline bool MarkRef<T>::try_removed(Node<T>* node, bool removed)
{
	return cas(node, false, node, true);
}

template<typename T>
inline Node<T>* MarkRef<T>::get_n_mark(bool* removed)
{
	uint64_t current_next{ address };

	*removed = (current_next & 1) == 1;

	return reinterpret_cast<Node<T>*>(current_next & 0xFFFFFFFFFFFFFFFE);
}

template<typename T>
inline bool MarkRef<T>::cas(Node<T>* old_ptr, bool old_mark, Node<T>* new_ptr, bool new_mark)
{
	uint64_t old_next{ reinterpret_cast<uint64_t>(old_ptr) };
	if (old_mark == true)
		++old_next;

	uint64_t new_next{ reinterpret_cast<uint64_t>(new_ptr) };
	if (new_mark == true)
		++new_next;

	return std::atomic_compare_exchange_strong(reinterpret_cast<std::atomic_uint64_t*>(&address), &old_next, new_next);
}
