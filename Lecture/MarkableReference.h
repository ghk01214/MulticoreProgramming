#pragma once

#include "pch.h"
#include "Node.h"

template<typename T>
class Node;

template<typename T>
class MarkableReference
{
public:
	MarkableReference();
	MarkableReference(bool mark, Node<T>* next);

	Node<T>* get_ptr() { return reinterpret_cast<Node<T>*>(address & 0xFFFFFFFFFFFFFFFE); }
	bool get_removed() { return (address & 1) == 1; }
	Node<T>* get_ptr_n_mark(bool* removed);

	bool cas(Node<T>* old_ptr, Node<T>* new_ptr, bool old_mark, bool new_mark);

private:
	uint64_t address;
};

template<typename T>
inline MarkableReference<T>::MarkableReference() :
	address{ 0 }
{
}

template<typename T>
inline MarkableReference<T>::MarkableReference(bool mark, Node<T>* addr)
{
	address = reinterpret_cast<uint64_t>(addr);

	if (mark == true)
		address = address | 1;
}

template<typename T>
inline Node<T>* MarkableReference<T>::get_ptr_n_mark(bool* removed)
{
	uint64_t current_next{ address };

	*removed = (current_next & 1) == 1;

	return reinterpret_cast<Node<T>*>(current_next & 0xFFFFFFFFFFFFFFFE);
}

template<typename T>
inline bool MarkableReference<T>::cas(Node<T>* old_ptr, Node<T>* new_ptr, bool old_mark, bool new_mark)
{
	uint64_t old_next{ reinterpret_cast<uint64_t>(old_ptr) };
	if (old_mark == true)
		++old_next;

	uint64_t new_next{ reinterpret_cast<uint64_t>(new_ptr) };
	if (new_mark == true)
		++new_next;

	return std::atomic_compare_exchange_strong(reinterpret_cast<std::atomic_uint64_t*>(&address), &old_next, new_next);
}
