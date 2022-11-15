#pragma once

#include "SNode.hpp"

template<typename T>
class Stack
{
public:
	Stack();
	~Stack() = default;

	void push(T data);
	T pop();
	void clear();

	void Print();

private:
	SNode<T>* _top;

	std::shared_mutex _lock;
};

template<typename T>
inline Stack<T>::Stack() :
	_top{ new SNode<T>{ -1 } }
{
}

template<typename T>
inline void Stack<T>::push(T data)
{
	SNode<T>* node{ new SNode<T>{ data } };

	_lock.lock();

	node->next = _top;
	_top = node;

	_lock.unlock();
}

template<typename T>
inline T Stack<T>::pop()
{
	_lock.lock();

	if (_top == nullptr)
	{
		_lock.unlock();
		return -2;
	}

	T data{ _top->data };
	SNode<T>* ptr{ _top };

	_top = _top->next;

	_lock.unlock();
	delete ptr;

	return data;
}

template<typename T>
inline void Stack<T>::clear()
{
	//SNode<T>* node{ _top };

	while (_top != nullptr)
	{
		SNode<T>* temp{ _top };
		_top = _top->next;

		delete temp;
	}
}

template<typename T>
inline void Stack<T>::Print()
{
	SNode<T>* node{ _top };

	for (int32_t i = 0; i < 20; ++i)
	{
		std::cout << std::format("{}, ", node->data);
		node = node->next;
	}

	std::cout << std::endl;
}
