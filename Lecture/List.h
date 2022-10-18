#pragma once

#include "pch.h"
#include "Node.h"

template<typename T>
class Node;

constexpr int32_t MAX_THREADS{ 16 };
template<typename T>
thread_local std::vector<Node<T>*> retired;
std::atomic_uint64_t reservations[MAX_THREADS];

std::atomic_uint32_t epoch{ 0 };
int32_t R;
int32_t num_thread;
thread_local int32_t thread_id;

template<typename T>
void Retire(Node<T>* ptr);
void StartOp();
void EndOp();

template<typename T>
class List
{
private:
	using Node = Node<T>;
public:
	List();
	~List();

	bool insert(T value);
	bool remove(T value);
	bool contains(T value);
	void clear();

	void Print();
private:
	void Find(T value, Node*& prev, Node*& current);

private:
	Node _head;
	Node _tail;
};

template<typename T>
List<T>::List()
{
	_head.data = std::numeric_limits<int>::min();
	_tail.data = std::numeric_limits<int>::max();

	_head.next = MarkRef<T>{ &_tail, false };
}

template<typename T>
List<T>::~List()
{
}

template<typename T>
inline bool List<T>::insert(T value)
{
	StartOp();

	Node* prev;
	Node* current;

	while (true)
	{
		Find(value, prev, current);

		if (current->data == value)
		{
			EndOp();
			return false;
		}

		Node* node{ new Node{ value, current } };

		if (prev->next.cas(current, false, node, false) == true)
			continue;

		EndOp();

		return true;
	}
}

template<typename T>
inline bool List<T>::remove(T value)
{
	StartOp();

	Node* prev;
	Node* current;

	while (true)
	{
		Find(value, prev, current);

		if (current->data != value)
		{
			EndOp();
			return false;
		}

		Node* success{ current->next.get() };

		if (current->next.try_removed(success, true) == false)
			continue;

		if (prev->next.cas(current, false, success, false) == true)
			Retire(current);

		EndOp();

		return true;
	}
}

template<typename T>
inline bool List<T>::contains(T value)
{
	StartOp();

	Node* current{ &_head };

	while (current->data < value)
	{
		current = current->next.get();
	}

	bool ret{ current->next.removed() == false and current->data == value };

	EndOp();

	return ret;
}

template<typename T>
inline void List<T>::clear()
{
	Node* node{ _head.next.get() };

	while (node != &_tail)
	{
		Node* temp{ node };
		node = node->next.get();

		delete temp;
	}

	_head.next = MarkRef<T>{ &_tail, false };
}

template<typename T>
inline void List<T>::Print()
{
	Node* node{ _head.next.get() };

	for (int32_t i = 0; i < 20; ++i)
	{
		std::cout << std::format("{}, ", node->data);

		if (node == &_tail)
			break;

		node = node->next.get();
	}

	std::cout << std::endl;
}

template<typename T>
inline void List<T>::Find(T value, Node*& prev, Node*& current)
{
	while (true)
	{
	retry:
		prev = &_head;
		current = prev->next.get();

		while (true)
		{
			bool removed;
			Node* success{ current->next.get_n_mark(&removed) };

			while (removed == true)
			{
				if (prev->next.cas(current, false, success, false) == false)
					goto retry;

				Retire(current);

				current = success;
				success = current->next.get_n_mark(&removed);
			}

			if (current->data >= value)
				return;

			prev = current;
			current = success;
		}
	}
}

//=====================================================================

template<typename T>
void Empty(Node<T>* ptr)
{
	uint64_t max_safe_epoch{ 0xFFFFFFFFFFFFFFFF };

	for (int32_t i = 0; i < num_thread; ++i)
	{
		max_safe_epoch = std::min(max_safe_epoch, reservations[i].load(std::memory_order_relaxed));
	}

	int32_t size{ static_cast<int32_t>(retired<T>.size()) };
	int32_t last{ size - 1 };
	int32_t index{ 0 };

	for (int32_t i = 0; i < size; ++i)
	{
		if (retired<T>[index]->retired_epoch < max_safe_epoch)
		{
			delete retired<T>[index];

			retired<T>[index] = retired<T>[last--];
			retired<T>.pop_back();
		}
		else
		{
			++index;
		}
	}
}

template<typename T>
void Retire(Node<T>* ptr)
{
	retired<T>.push_back(ptr);
	ptr->retired_epoch = epoch.load(std::memory_order_relaxed);

	if (retired<T>.size() >= R)
		Empty(ptr);
}

void StartOp()
{
	reservations[thread_id].store(epoch.fetch_add(1, std::memory_order_relaxed), std::memory_order_relaxed);
}

void EndOp()
{
	reservations[thread_id].store(0xFFFFFFFFFFFFFFFF, std::memory_order_relaxed);
}
