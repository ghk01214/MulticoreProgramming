#pragma once

#include "SLNode.h"

template<typename T>
class SkipList
{
public:
	SkipList();
	~SkipList();

	bool insert(T value);
	bool remove(T value);
	bool contains(T value);
	void clear();

	void Print();
private:
	void Find(T value, SLNode<T>* prev[], SLNode<T>* current[]);

private:
	SLNode<T> _head;
	SLNode<T> _tail;

	std::uniform_int_distribution<int32_t> uid_level;
	std::shared_mutex lock;
};

template<typename T>
SkipList<T>::SkipList() :
	uid_level{ 0, 1 }
{
	_head.data = _I32_MAX;		// == std::numeric_limits<int32_t>::min()	// defined in <limits> header
	_tail.data = _I32_MIN;		// == std::numeric_limits<int32_t>::max()	// defined in <limits> header

	_head.top_level = MAX_LEVEL;
	_tail.top_level = MAX_LEVEL;

	for (auto& node : _head.next)
	{
		node = &_tail;
	}
}

template<typename T>
SkipList<T>::~SkipList()
{
}

template<typename T>
inline bool SkipList<T>::insert(T value)
{
	SLNode<T>* prev[MAX_LEVEL + 1];
	SLNode<T>* current[MAX_LEVEL + 1];

	lock.lock();
	Find(value, prev, current);

	if (current[0]->data == value)
	{
		lock.unlock();
		return false;
	}

	int32_t new_level{ 0 };
	extern std::default_random_engine dre;

	for (int32_t i = 0; i < MAX_LEVEL; ++i)
	{
		if (uid_level(dre) == 0)
			continue;

		++new_level;
	}

	SLNode<T>* node{ new SLNode<T>{ value, new_level } };

	for (int32_t i = 0; i <= new_level; ++i)
	{
		node->next[i] = current[i];
		prev[i]->next[i] = node;
	}
	
	lock.unlock();
	return true;
}

template<typename T>
inline bool SkipList<T>::remove(T value)
{
	SLNode<T>* prev[MAX_LEVEL + 1];
	SLNode<T>* current[MAX_LEVEL + 1];

	lock.lock();
	Find(value, prev, current);

	if (current[0]->data != value)
	{
		lock.unlock();
		return false;
	}
	
	for (int32_t i = 0; i <= current[0]->top_level; ++i)
	{
		prev[i]->next[i] = current[i]->next[i];
	}

	delete current[0];
	lock.unlock();

	return true;
}

template<typename T>
inline bool SkipList<T>::contains(T value)
{
	SLNode<T>* prev[MAX_LEVEL + 1];
	SLNode<T>* current[MAX_LEVEL + 1];

	lock.lock();
	Find(value, prev, current);

	if (current[0]->data != value)
	{
		lock.unlock();
		return false;
	}

	lock.unlock();
	return true;
}

template<typename T>
inline void SkipList<T>::clear()
{
	SLNode<T>* node{ _head.next[0] };

	while (node != &_tail)
	{
		SLNode<T>* temp{ node };
		node = node->next[0];

		delete temp;
	}

	for (auto& n : _head.next)
	{
		n = &_tail;
	}
}

template<typename T>
inline void SkipList<T>::Print()
{
	SLNode<T>* node{ _head.next[0] };

	for (int32_t i = 0; i < 20; ++i)
	{
		if (node == &_tail)
			break;

		std::cout << std::format("{}, ", node->data);
		node = node->next[0];
	}

	std::cout << std::endl;
}

template<typename T>
inline void SkipList<T>::Find(T value, SLNode<T>* prev[], SLNode<T>* current[])
{
	prev[MAX_LEVEL] = &_head;

	for (int32_t i = MAX_LEVEL; i >= 0; --i)
	{
		current[i] = prev[i]->next[i];

		while (current[i]->data < value)
		{
			prev[i] = current[i];
			current[i] = current[i]->next[i];
		}

		if (i == 0)
			break;

		prev[i - 1] = prev[i];
	}
}
