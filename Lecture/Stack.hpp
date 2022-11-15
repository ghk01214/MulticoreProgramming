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
	bool cas(SNode<T>* volatile* next, SNode<T>* old_ptr, SNode<T>* new_ptr);

private:
	SNode<T>* volatile _top;
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

	while (true)
	{
		SNode<T>* ptr{ _top };
		node->next = ptr;

		if (cas(&_top, ptr, node) == true)
			return;
	}
}

template<typename T>
inline T Stack<T>::pop()
{
	while (true)
	{
		SNode<T>* node{ _top };

		if (node != _top)
			continue;

		if (node == nullptr)
			return -2;

		T data{ node->data };
		SNode<T>* next{ node->next };

		if (cas(&_top, node, next) == false)
			continue;

		//delete node;
		return data;
	}
}

template<typename T>
inline void Stack<T>::clear()
{
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

template<typename T>
inline bool Stack<T>::cas(SNode<T>* volatile* next, SNode<T>* old_ptr, SNode<T>* new_ptr)
{
	return std::atomic_compare_exchange_strong(
		reinterpret_cast<volatile std::atomic_int64_t*>(next),
		reinterpret_cast<int64_t*>(&old_ptr),
		reinterpret_cast<int64_t>(new_ptr)
	);
}
