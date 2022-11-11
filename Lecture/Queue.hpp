#pragma once

#include "QNode.hpp"

template<typename T>
class Queue
{
public:
	Queue();
	~Queue() = default;

	void push(T data);
	T pop();
	void clear();

	void Print();
private:
	bool cas(QNode<T>* volatile* next, QNode<T>* old_p, QNode<T>* new_p)
	{
		return std::atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic_int64_t*>(next),
			reinterpret_cast<int64_t*>(&old_p),
			reinterpret_cast<int64_t>(new_p));
	}

private:
	QNode<T>* volatile _head;
	QNode<T>* volatile _tail;
};

template<typename T>
Queue<T>::Queue() :
	_head{ new QNode<T>{ -1 } },
	_tail{ _head }
{
}

template<typename T>
inline void Queue<T>::push(T data)
{
	QNode<T>* node{ new QNode<T>{ data } };

	while (true)
	{
		QNode<T>* last{ _tail };
		QNode<T>* next{ last->next };

		if (last != _tail)
			continue;

		if (next != nullptr)
		{
			cas(&_tail, last, next);
			continue;
		}

		if (cas(&last->next, nullptr, node) == false)
			continue;
		
		cas(&_tail, last, node);
		return;
	}
}

template<typename T>
inline T Queue<T>::pop()
{
	while (true)
	{
		QNode<T>* first{ _head };
		QNode<T>* next{ first->next };
		QNode<T>* last{ _tail };

		if (first != _head)
			continue;

		if (next == nullptr)
			return -1;

		if (first == last)
		{
			cas(&_tail, last, next);
			continue;
		}

		T value{ next->data };

		if (cas(&_head, first, next) == false)
			continue;

		delete first;
		return value;
	}
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

	_head->next = nullptr;
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
