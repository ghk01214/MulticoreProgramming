#pragma once

#include "SNode.hpp"
#include "Eliminator.hpp"

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
	Eliminator<T> _eliminator;
};

template<typename T>
inline Stack<T>::Stack() :
	_top{ new SNode<T>{ -1 } },
	_eliminator{}
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

		if (_eliminator.visit(data) == -1)
			return;
	}
}

template<typename T>
inline T Stack<T>::pop()
{
	while (true)
	{
		SNode<T>* node{ _top };

		if (node == nullptr)
			return -2;

		T data{ node->data };
		SNode<T>* next{ node->next };

		if (node != _top)
			continue;

		if (cas(&_top, node, next) == true)
			return data;

		int64_t ret{ _eliminator.visit(-1) };
		
		if (ret != -1)
			return ret;
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
#if _WIN64
		reinterpret_cast<volatile std::atomic_int64_t*>(next),
		reinterpret_cast<int64_t*>(&old_ptr),
		reinterpret_cast<int64_t>(new_ptr)
#else
		reinterpret_cast<volatile std::atomic_int32_t*>(next),
		reinterpret_cast<int32_t*>(&old_ptr),
		reinterpret_cast<int32_t>(new_ptr)
#endif
	);
}
