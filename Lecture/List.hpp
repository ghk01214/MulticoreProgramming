#pragma once

#include "MarkablePtr.h"
#include "LNode.h"

template<typename T>
class List
{
public:
	List();
	~List();

	bool insert(T value);
	bool remove(T value);
	bool contains(T value);
	void clear();

	void Print();
private:
	void Find(T value, LNode<T>*& prev, LNode<T>*& current);

private:
	LNode<T> _head;
	LNode<T> _tail;
};

template<typename T>
List<T>::List()
{
	_head.data = _I32_MIN;	// == std::numeric_limits<int32_t>::min();	// defined in <limits> header
	_tail.data = _I32_MAX;	// == std::numeric_limits<int32_t>::max();	// defined in <limits> header

	_head.next = MarkablePtr<T>{ false, &_tail };
}

template<typename T>
List<T>::~List()
{
}

template<typename T>
inline bool List<T>::insert(T value)
{
	while (true)
	{
		LNode<T>* prev;
		LNode<T>* current;

		Find(value, prev, current);

		if (current->data == value)
			return false;

		LNode<T>* node{ new LNode<T>{ value, current } };

		if (prev->next.cas(current, node, false, false) == true)
			return true;

		delete node;
	}
}

template<typename T>
inline bool List<T>::remove(T value)
{
	while (true)
	{
		LNode<T>* prev;
		LNode<T>* current;

		Find(value, prev, current);

		if (current->data != value)
			return false;
		
		LNode<T>* success{ current->next.get_ptr() };

		if (current->next.try_change_mark(success, true) == false)
			continue;

		prev->next.cas(current, success, false, false);

		return true;
	}
}

template<typename T>
inline bool List<T>::contains(T value)
{
	LNode<T>* current{ _head.next.get_ptr() };
	bool removed;

	while (current->data < value)
	{
		current = current->next.get_ptr();
		removed = current->next.get_removed();
	}
	
	return current->data == value and removed == false;
}

template<typename T>
inline void List<T>::clear()
{
	LNode<T>* node{ _head.next.get_ptr() };

	while (node != &_tail)
	{
		LNode<T>* temp{ node };
		node = node->next.get_ptr();

		delete temp;
	}

	_head.next = MarkablePtr<T>{ false, &_tail };
}

template<typename T>
inline void List<T>::Print()
{
	LNode<T>* node{ _head.next.get_ptr() };

	for (int32_t i = 0; i < 20; ++i)
	{
		if (node != &_tail)
		{
			std::cout << std::format("{}, ", node->data);
			node = node->next.get_ptr();
		}
	}

	std::cout << std::endl;
}

template<typename T>
inline void List<T>::Find(T value, LNode<T>*& prev, LNode<T>*& current)
{
	while (true)
	{
	retry:
		prev = &_head;
		current = prev->next.get_ptr();

		while (true)
		{
			bool removed;
			LNode<T>* success{ current->next.get_ptr_n_mark(&removed) };

			while (removed == true)
			{
				if (prev->next.cas(current, success, false, false) == false)
					goto retry;

				current = success;
				success = current->next.get_ptr_n_mark(&removed);
			}

			if (current->data >= value)
				return;

			prev = current;
			current = success;
		}
	}
}
