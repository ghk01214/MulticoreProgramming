#pragma once

#include "QNode.hpp"

template<typename T>
class Queue
{
public:
	Queue();
	~Queue();

	void push(T data);
	T pop();
	void clear();

	void Print();

private:
	QNode<T>* _head;
	QNode<T>* _tail;

	std::shared_mutex lock_push;
	std::shared_mutex lock_pop;
};

template<typename T>
Queue<T>::Queue() :
	_head{ new QNode<T>{ -1 } },
	_tail{ _head }
{
	//_head = _tail = new QNode<T>{ -1 };
}

template<typename T>
Queue<T>::~Queue()
{
}

template<typename T>
inline void Queue<T>::push(T data)
{
	lock_push.lock();

	QNode<T>* node{ new QNode<T>{ data } };

	_tail->next = node;
	_tail = node;

	lock_push.unlock();
}

template<typename T>
inline T Queue<T>::pop()
{
	T result;

	lock_pop.lock();

	if (_head->next == nullptr)
	{
		lock_pop.unlock();
		return false;
	}

	result = _head->next->data;
	QNode<T>* temp{ _head };
	_head = _head->next;

	lock_pop.unlock();
	delete temp;

	return result;
}

template<typename T>
inline void Queue<T>::clear()
{
	QNode<T>* node{ _head->next };

	while (node != nullptr)
	{
		QNode<T>* temp{ node };
		node = node->next;

		delete temp;
	}

	_tail = _head;
}

template<typename T>
inline void Queue<T>::Print()
{
	QNode<T>* node{ _head->next };

	for (int32_t i = 0; i < 20; ++i)
	{
		std::cout << std::format("{}, ", node->data);
		node = node->next;
	}

	std::cout << std::endl;
}
