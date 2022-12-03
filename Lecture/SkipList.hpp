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

	while (true)
	{
		int32_t found{ Find(value, prev, current) };

		// value 값이 존재한다
		if (found != -1)
		{
			if (current[found]->removed == true)
				continue;

			while (current[found]->fully_linked == false) {}

			return false;
		}

		// value가 존재하지 않는다
		// lock을 하고 valid 검사를 한다
		// 전부 lock을 하고 valid 검사를 하면 필요없는 lock을 추가로 할 수 있으니,
		// 하나씩 lock을 하면서 검사한다

		int32_t new_level{ 0 };
		extern std::default_random_engine dre;

		for (int32_t i = 0; i < MAX_LEVEL; ++i)
		{
			if (uid_level(dre) == 0)
				break;

			++new_level;
		}

		bool valid{ true };
		int32_t locked_top_level{ 0 };

		for (int32_t i = 0; i <= new_level; ++i)
		{
			prev[i]->lock();

			locked_top_level = i;
			valid =
			{
				prev[i]->removed == false
				and current[i]->removed == false
				and prev[i]->next[i] == current[i]
			};

			if (valid == false)
				break;
		}

		if (valid == false)
		{
			for (int32_t i = 0; i <= locked_top_level; ++i)
			{
				prev[i]->unlock();
			}

			continue;
		}

		SLNode<T>* node{ new SLNode<T>{ value, new_level } };

		for (int32_t i = 0; i <= new_level; ++i)
		{
			node->next[i] = current[i];
		}

		for (int32_t i = 0; i <= new_level; ++i)
		{
			prev[i]->next[i] = node;
		}

		node->fully_linked = true;

		for (int32_t i = 0; i <= locked_top_level; ++i)
		{
			prev[i]->unlock();
		}

		return true;
	}
}

template<typename T>
inline bool SkipList<T>::remove(T value)
{
	SLNode<T>* prev[MAX_LEVEL + 1];
	SLNode<T>* current[MAX_LEVEL + 1];

	bool removed{ false };
	int32_t new_level{ -1 };
	SLNode<T>* node{ nullptr };

	while (true)
	{
		int32_t found{ Find(value, prev, current) };

		if (found != -1)
			node = current[found];

		if (removed == false
			and (found == -1
				or node->fully_linked == false
				or node->top_level != found
				or node->removed == true))
			return false;

		if (removed == false)
		{
			new_level = node->top_level;

			node->lock();

			if (node->removed == true)
			{
				node->unlock();
				return false;
			}

			node->removed = true;
			removed = true;
		}

		bool valid{ true };
		int32_t locked_top_level{ -1 };

		for (int32_t i = 0; i <= new_level; ++i)
		{
			prev[i]->lock();

			locked_top_level = i;
			valid = prev[i]->removed == false and prev[i]->next[i] == node;

			if (valid == false)
				break;
		}

		if (valid == false)
		{
			for (int32_t i = 0; i <= locked_top_level; ++i)
			{
				prev[i]->unlock();
			}

			continue;
		}

		for (int32_t i = new_level; i >= 0; --i)
		{
			prev[i]->next[i] = node->next[i];
		}

		for (int32_t i = 0; i <= locked_top_level; ++i)
		{
			prev[i]->unlock();
		}

		if (found != -1)
			node->unlock();

		return true;
	}
}

template<typename T>
inline bool SkipList<T>::contains(T value)
{
	SLNode<T>* prev[MAX_LEVEL + 1];
	SLNode<T>* current[MAX_LEVEL + 1];

	int32_t found{ Find(value, prev, current) };

	return found != -1 and current[found]->fully_linked == true and current[found]->removed == false;
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
inline int32_t SkipList<T>::Find(T value, SLNode<T>** prev, SLNode<T>** current)
{
	int32_t level{ -1 };

	prev[MAX_LEVEL] = &_head;

	for (int32_t i = MAX_LEVEL; i >= 0; --i)
	{
		current[i] = prev[i]->next[i];

		while (current[i]->data < value)
		{
			prev[i] = current[i];
			current[i] = current[i]->next[i];
		}

		if (current[i]->data == value and level == -1)
			level = i;

		if (i == 0)
			break;

		prev[i - 1] = prev[i];
	}

	return level;
}
