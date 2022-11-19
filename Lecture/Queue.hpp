#pragma once

#include "StampPtr.hpp"
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
	bool cas(StampPtr<T>* next, QNode<T>* old_ptr, int64_t old_stamp, QNode<T>* new_ptr, int64_t new_stamp);

private:
	StampPtr<T> _head;
	StampPtr<T> _tail;
};

template<typename T>
Queue<T>::Queue() :
	_head{ nullptr },
	_tail{ nullptr }
{
	QNode<T>* node{ new QNode<T>{ -1 } };

	_head.set(node);
	_tail.set(node);
}

template<typename T>
inline void Queue<T>::push(T data)
{
	QNode<T>* node{ new QNode<T>{ data } };

	while (true)
	{
		StampPtr<T> last{ _tail };
		StampPtr<T> next{ last.ptr->next };

		if (last.stamp != _tail.stamp)
			continue;

		if (next.ptr != nullptr)
		{
			cas(&_tail, last.ptr, last.stamp, next.ptr, last.stamp + 1);
			continue;
		}

		if (cas(&last.ptr->next, nullptr, next.stamp, node, next.stamp + 1) == true)
		{
			cas(&_tail, last.ptr, last.stamp, node, last.stamp + 1);
			return;
		}
	}
}

template<typename T>
inline T Queue<T>::pop()
{
	while (true)
	{
		StampPtr<T> first{ _head };
		StampPtr<T> next{ first.ptr->next };
		StampPtr<T> last{ _tail };

		if (first.stamp != _head.stamp)
			continue;

		if (next.ptr == nullptr)
			return -1;

		if (first.stamp == last.stamp)
		{
			cas(&_tail, last.ptr, last.stamp, next.ptr, last.stamp + 1);
			continue;
		}

		T value{ next.ptr->data };

		if (cas(&_head, first.ptr, first.stamp, next.ptr, first.stamp + 1) == false)
		{
			first.release();
			return value;
		}
	}
}

template<typename T>
inline void Queue<T>::clear()
{
	while (_head.get()->next.get() != nullptr)
	{
		QNode<T>* node{ _head.get() };

		_head.set(_head.get()->next.get());
		delete node;
	}

	_tail.ptr = _head.ptr;
}

template<typename T>
inline void Queue<T>::Print()
{
	StampPtr<T> node{ _head.ptr->next };

	for (int32_t i = 0; i < 20; ++i)
	{
		if (node.ptr == nullptr)
			break;

		std::cout << std::format("{}, ", node.ptr->data);
		node = node.ptr->next;
	}

	std::cout << std::endl;
}

template<typename T>
inline bool Queue<T>::cas(StampPtr<T>* next, QNode<T>* old_ptr, int64_t old_stamp, QNode<T>* new_ptr, int64_t new_stamp)
{
	StampPtr<T> old_st{ old_ptr, old_stamp };

	return InterlockedCompareExchange128(
		reinterpret_cast<int64_t volatile*>(next),
		new_stamp,
		reinterpret_cast<int64_t>(new_ptr),
		reinterpret_cast<int64_t*>(&old_st)
	);
}
