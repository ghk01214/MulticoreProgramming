#pragma once

#include "pch.h"
#include "Node.h"

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
	void Find(T value, Node<T>*& prev, Node<T>*& current);

private:
	Node<T> _head;
	Node<T> _tail;
};

template<typename T>
List<T>::List()
{
	_head.data = std::numeric_limits<int>::min();
	_tail.data = std::numeric_limits<int>::max();

	_head.next = MarkableReference<T>{ false, &_tail };
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
		Node<T>* prev;
		Node<T>* current;

		Find(value, prev, current);

		if (current->data == value)
			return false;

		Node<T>* node{ new Node<T>{ value, current } };

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
		Node<T>* prev;
		Node<T>* current;

		Find(value, prev, current);

		if (current->data != value)
			return false;
		
		Node<T>* success{ current->next.get_ptr() };

		if (current->next.try_change_mark(success, true) == false)
			continue;

		prev->next.cas(current, success, false, false);

		return true;
	}
}

template<typename T>
inline bool List<T>::contains(T value)
{
	Node<T>* current{ _head.next.get_ptr() };
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
	Node<T>* node{ _head.next.get_ptr() };

	while (node != &_tail)
	{
		Node<T>* temp{ node };
		node = node->next.get_ptr();

		delete temp;
	}

	_head.next = MarkableReference<T>{ false, &_tail };
}

template<typename T>
inline void List<T>::Print()
{
	Node<T>* node{ _head.next.get_ptr() };

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
inline void List<T>::Find(T value, Node<T>*& prev, Node<T>*& current)
{
	while (true)
	{
	retry:
		prev = &_head;
		current = prev->next.get_ptr();

		while (true)
		{
			bool removed;
			Node<T>* success{ current->next.get_ptr_n_mark(&removed) };

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
