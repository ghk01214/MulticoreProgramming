#pragma once

template<typename T>
class MarkablePtr;

template<typename T>
class LNode
{
public:
	LNode();
	LNode(T data);
	LNode(T data, LNode<T>* next);

public:
	T data;
	MarkablePtr<T> next;
};

template<typename T>
inline LNode<T>::LNode() :
	next{ false, nullptr }
{
}

template<typename T>
inline LNode<T>::LNode(T data) :
	data{ data },
	next{ false, nullptr }
{
}

template<typename T>
inline LNode<T>::LNode(T data, LNode<T>* next) :
	data{ data },
	next{ false, next }
{
}
