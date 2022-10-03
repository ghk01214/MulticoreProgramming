#pragma once

template<typename T>
struct Node
{
	T data;
	Node<T>* next;

	Node() : next{nullptr} {}
	Node(T data) : data{ data } {}
};

template<typename T>
class List
{
public:
	List();
	~List();

	bool insert(T value);
	bool remove(T value);
	bool contains(T value);
	void clear();

	void Print();

private:
	Node<T> _head;
	Node<T> _tail;
};

template<typename T>
List<T>::List()
{
	_head.data = std::numeric_limits<int>::min();
	_tail.data = std::numeric_limits<int>::max();

	_head.next = &_tail;
}

template<typename T>
List<T>::~List()
{
}

template<typename T>
inline bool List<T>::insert(T value)
{
	Node<T>* prev{ &_head };
	Node<T>* current{ prev->next };

	while (current->data < value)
	{
		prev = current;
		current = current->next;
	}

	if (current->data != value)
	{
		Node<T>* node{ new Node<T>{ value } };

		node->next = current;
		prev->next = node;

		return true;
	}

	return false;
}

template<typename T>
inline bool List<T>::remove(T value)
{
	Node<T>* prev{ &_head };
	Node<T>* current{ prev->next };

	while (current->data < value)
	{
		prev = current;
		current = current->next;
	}

	if (current->data != value)
		return false;

	prev->next = current->next;

	delete current;

	return true;
}

template<typename T>
inline bool List<T>::contains(T value)
{
	Node<T>* prev{ &_head };
	Node<T>* current{ prev->next };

	while (current->data < value)
	{
		prev = current;
		current = current->next;
	}

	return current->data == value;
}

template<typename T>
inline void List<T>::clear()
{
	Node<T>* p{ _head.next };

	while (p != &_tail)
	{
		Node<T>* t{ p };

		p = p->next;
		delete t;
	}

	_head.next = &_tail;
}

template<typename T>
inline void List<T>::Print()
{
	Node<T>* p{ _head.next };

	for (int32_t i = 0; i < 20; ++i)
	{
		if (p != &_tail)
		{
			std::cout << std::format("{}, ", p->data);
			p = p->next;
		}
	}

	std::cout << std::endl;
}
