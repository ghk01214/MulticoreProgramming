#pragma once

#include "pch.h"

template<typename T>
class Node
{
public:
	Node() : next{ nullptr }, removed{ false } {}
	Node(T data) : data{ data }, next{ nullptr }, removed{ false } {}

	void lock() { _lock.lock(); }
	void unlock() { _lock.unlock(); }

private:
	std::mutex _lock;
public:
	T data;
	Node<T>* volatile next;
	volatile bool removed;
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
	bool validate(Node<T>* prev, Node<T>* current);

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
	_tail.next = nullptr;
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
		Node<T>* prev{ &_head };
		Node<T>* current{ prev->next };

		while (current->data < value)
		{
			prev = current;
			current = current->next;
		}

		prev->lock();
		current->lock();

		if (validate(prev, current) == false)
		{
			prev->unlock();
			current->unlock();

			continue;
		}

		if (current->data == value)
		{
			prev->unlock();
			current->unlock();

			return false;
		}

		Node<T>* node{ new Node<T>{ value } };

		node->next = current;
		prev->next = node;

		prev->unlock();
		current->unlock();

		return true;
	}
}

template<typename T>
inline bool List<T>::remove(T value)
{
	while (true)
	{
		Node<T>* prev{ &_head };
		Node<T>* current{ prev->next };

		while (current->data < value)
		{
			prev = current;
			current = current->next;
		}

		prev->lock();
		current->lock();

		if (validate(prev, current) == false)
		{
			prev->unlock();
			current->unlock();

			continue;
		}

		if (current->data != value)
		{
			prev->unlock();
			current->unlock();

			return false;
		}

		current->removed = true;
		prev->next = current->next;

		prev->unlock();
		current->unlock();

		return true;
	}
}

template<typename T>
inline bool List<T>::contains(T value)
{
	Node<T>* current{ &_head };

	while (current->data < value)
	{
		current = current->next;
	}

	return current->data == value and current->removed == false;
}

template<typename T>
inline void List<T>::clear()
{
	Node<T>* node{ _head.next };

	while (node != &_tail)
	{
		Node<T>* temp{ node };
		node = node->next;

		delete temp;
	}

	_head.next = &_tail;
}

template<typename T>
inline void List<T>::Print()
{
	Node<T>* node{ _head.next };

	for (int32_t i = 0; i < 20; ++i)
	{
		if (node != &_tail)
		{
			std::cout << std::format("{}, ", node->data);
			node = node->next;
		}
	}

	std::cout << std::endl;
}

template<typename T>
inline bool List<T>::validate(Node<T>* prev, Node<T>* current)
{
	return prev->removed == false && current->removed == false && prev->next == current;
}
