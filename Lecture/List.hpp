#pragma once

#include "pch.h"
#include "List.h"

template<typename T>
List<T>::List()
{
	_head.data = std::numeric_limits<int>::min();
	_tail.data = std::numeric_limits<int>::max();

	_head.next = &_tail;
}

template<typename T>
List<T>::~List()
{
}

template<typename T>
inline bool List<T>::insert(T value)
{
	Node<T>* prev{ &_head };

	ml.lock();
	Node<T>* current{ prev->next };

	while (current->data < value)
	{
		prev = current;
		current = current->next;
	}

	if (current->data != value)
	{
		Node<T>* node{ new Node<T>{ value } };

		node->next = current;
		prev->next = node;

		ml.unlock();
		return true;
	}

	ml.unlock();
	return false;
}

template<typename T>
inline bool List<T>::remove(T value)
{
	Node<T>* prev{ &_head };
	ml.lock();
	Node<T>* current{ prev->next };

	while (current->data < value)
	{
		prev = current;
		current = current->next;
	}

	if (current->data != value)
	{
		ml.unlock();
		return false;
	}

	prev->next = current->next;

	delete current;

	ml.unlock();
	return true;
}

template<typename T>
inline bool List<T>::contains(T value)
{
	Node<T>* prev{ &_head };
	ml.lock();
	Node<T>* current{ prev->next };

	while (current->data < value)
	{
		prev = current;
		current = current->next;
	}

	if (current->data != value)
	{
		ml.unlock();
		return false;
	}

	ml.unlock();
	return true;
}

template<typename T>
inline void List<T>::clear()
{
	Node<T>* p{ _head.next };

	while (p != &_tail)
	{
		Node<T>* t{ p };

		p = p->next;
		delete t;
	}

	_head.next = &_tail;
}

template<typename T>
inline void List<T>::Print()
{
	Node<T>* p{ _head.next };

	for (int32_t i = 0; i < 20; ++i)
	{
		if (p != &_tail)
		{
			std::cout << std::format("{}, ", p->data);
			p = p->next;
		}
	}

	std::cout << std::endl;
}
