#pragma once

#include "SLNode.hpp"

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
	int32_t Find(T value, SLNode<T>** prev, SLNode<T>** current);

private:
	SLNode<T> _head;
	SLNode<T> _tail;

	std::uniform_int_distribution<int32_t> uid_level;
};

template<typename T>
SkipList<T>::SkipList() :
	uid_level{ 0, 1 }
{
	_head.data = _I32_MIN;		// == std::numeric_limits<int32_t>::min()	// defined in <limits> header
	_tail.data = _I32_MAX;		// == std::numeric_limits<int32_t>::max()	// defined in <limits> header

	_head.top_level = MAX_LEVEL;
	_tail.top_level = MAX_LEVEL;

	for (auto& node : _head.next)
	{
		node = SLMarkablePtr<T>{ false, &_tail };
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

	while (true)
	{
		int32_t found{ Find(value, prev, current) };

		while (found == -1)
		{
			found = Find(value, prev, current);
		}

		if (found == 1)
			return false;

		// value 값이 존재하지 않는다.

		int32_t new_level{ 0 };
		extern std::default_random_engine dre;

		for (int32_t i = 0; i < MAX_LEVEL; ++i)
		{
			if (uid_level(dre) == 0)
				break;

			++new_level;
		}

		SLNode<T>* node{ new SLNode<T>{value, new_level} };

		for (int32_t i = 0; i <= new_level; ++i)
		{
			node->next[i] = SLMarkablePtr<T>{ false, current[i] };
		}

		if (prev[0]->next[0].cas(current[0], false, node, false) == false)
			continue;

		for (int32_t i = 1; i <= new_level; ++i)
		{
			while (prev[i]->next[i].cas(current[i], false, node, false) == false)
			{
				Find(value, prev, current);
			}
		}
	}
}

template<typename T>
inline bool SkipList<T>::remove(T value)
{
	SLNode<T>* prev[MAX_LEVEL + 1];
	SLNode<T>* current[MAX_LEVEL + 1];

	SLNode<T>* node{ nullptr };

	while (true)
	{
		int32_t found{ Find(value, prev, current) };

		while (found == -1)
		{
			found = Find(value, prev, current);
		}

		if (found == 0)
			return false;

		bool cas_failed{ false };

		for (int32_t i = current[0]->top_level; i > 0; --i)
		{
			if (prev[i]->next[i].cas(current[i], false, current[i], true) == false)
			{
				cas_failed = true;
				break;
			}
		}

		if (cas_failed == true)
			continue;

		if (prev[0]->next[0].cas(current[0], false, current[0], true) == false)
			continue;

		Find(value, prev, current);

		return true;
	}
}

template<typename T>
inline bool SkipList<T>::contains(T value)
{
	SLNode<T>* prev[MAX_LEVEL + 1];
	SLNode<T>* current[MAX_LEVEL + 1];

	return Find(value, prev, current);
}

template<typename T>
inline void SkipList<T>::clear()
{
	SLNode<T>* node{ _head.next[0].get_ptr() };

	while (node != &_tail)
	{
		SLNode<T>* temp{ node };
		node = node->next[0].get_ptr();

		delete temp;
	}

	for (auto& n : _head.next)
	{
		n = SLMarkablePtr<T>{ false, &_tail };
	}
}

template<typename T>
inline void SkipList<T>::Print()
{
	SLNode<T>* node{ _head.next[0].get_ptr() };

	for (int32_t i = 0; i < 20; ++i)
	{
		if (node == &_tail)
			break;

		std::cout << std::format("{}, ", node->data);
		node = node->next[0].get_ptr();
	}

	std::cout << std::endl;
}

template<typename T>
inline int32_t SkipList<T>::Find(T value, SLNode<T>** prev, SLNode<T>** current)
{
	prev[MAX_LEVEL] = &_head;

	for (int32_t i = MAX_LEVEL; i >= 0; --i)
	{
		current[i] = prev[i]->next[i].get_ptr();

		while (true)
		{
			bool removed;
			SLNode<T>* node{ current[i]->next[i].get_ptr_n_mark(&removed) };

			while (removed == true)
			{
				if (prev[i]->next[i].cas(current[i], false, node, false) == false)
					return -1;

				current[i] = node;
				node = current[i]->next[i].get_ptr_n_mark(&removed);
			}

			if (current[i]->data >= value)
			{
				if (i == 0)
					return current[0]->data == value;

				prev[i - 1] = prev[i];

				break;
			}

			prev[i] = current[i];
			current[i] = node;
		}
	}
}
