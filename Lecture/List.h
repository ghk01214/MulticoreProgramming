#pragma once

#include "pch.h"

template<typename T>
class Node
{
public:
	Node() : next{ nullptr } {}
	Node(T data) : data{ data } {}

	void lock() { _lock.lock(); }
	void unlock() { _lock.unlock(); }

private:
	std::mutex _lock;
public:
	T data;
	Node<T>* next;
};

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
	Node<T> _head;
	Node<T> _tail;
};

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
	_head.lock();

	Node<T>* prev{ &_head };
	Node<T>* current{ prev->next };

	current->lock();

	while (current->data < value)
	{
		prev->unlock();

		prev = current;
		current = current->next;

		current->lock();
	}

	if (current->data != value)
	{
		Node<T>* node{ new Node<T>{ value } };

		node->next = current;
		prev->next = node;

		prev->unlock();
		current->unlock();

		return true;
	}

	prev->unlock();
	current->unlock();

	return false;
}

template<typename T>
inline bool List<T>::remove(T value)
{
	_head.lock();

	Node<T>* prev{ &_head };
	Node<T>* current{ prev->next };

	current->lock();

	while (current->data < value)
	{
		prev->unlock();

		prev = current;
		current = current->next;

		current->lock();
	}

	if (current->data != value)
	{
		prev->unlock();
		current->unlock();

		return false;
	}

	prev->next = current->next;

	current->unlock();

	delete current;

	prev->unlock();

	return true;
}

template<typename T>
inline bool List<T>::contains(T value)
{
	_head.lock();

	Node<T>* prev{ &_head };
	Node<T>* current{ prev->next };

	current->lock();

	while (current->data < value)
	{
		prev->unlock();

		prev = current;
		current = current->next;

		current->lock();
	}

	if (current->data != value)
	{
		prev->unlock();
		current->unlock();

		return false;
	}

	prev->unlock();
	current->unlock();

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
	//_head.lock();
	Node<T>* p{ _head.next };

	for (int32_t i = 0; i < 20; ++i)
	{
		if (p != &_tail)
		{
			//p->lock();
			std::cout << std::format("{}, ", p->data);
			p = p->next;
			//p->unlock();
		}
	}

	std::cout << std::endl;
}
