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
	std::shared_ptr<Node<T>> next;
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
	bool validate(std::shared_ptr<Node<T>> prev, std::shared_ptr<Node<T>> current);

private:
	std::shared_ptr<Node<T>> _head;
	std::shared_ptr<Node<T>> _tail;
};

template<typename T>
List<T>::List() :
	_head{ std::make_shared<Node<T>>(std::numeric_limits<int32_t>::min()) },
	_tail{ std::make_shared<Node<T>>(std::numeric_limits<int32_t>::max()) }
{
	_head->next = _tail;
	_tail->next = nullptr;
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
		std::shared_ptr<Node<T>> prev{ _head };
		std::shared_ptr<Node<T>> current{ prev->next };

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

		std::shared_ptr<Node<T>> node{ std::make_shared<Node<T>>(value) };

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
		std::shared_ptr<Node<T>> prev{ _head };
		std::shared_ptr<Node<T>> current{ prev->next };

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
	std::shared_ptr<Node<T>> current{ _head };

	while (current->data < value)
	{
		current = current->next;
	}

	return current->data == value and current->removed == false;
}

template<typename T>
inline void List<T>::clear()
{
	_head->next = _tail;
}

template<typename T>
inline void List<T>::Print()
{
	std::shared_ptr<Node<T>> node{ _head->next };

	for (int32_t i = 0; i < 20; ++i)
	{
		if (node != _tail)
		{
			std::cout << std::format("{}, ", node->data);
			node = node->next;
		}
	}

	std::cout << std::endl;
}

template<typename T>
inline bool List<T>::validate(std::shared_ptr<Node<T>> prev, std::shared_ptr<Node<T>> current)
{
	return prev->removed == false && current->removed == false && prev->next == current;
}
