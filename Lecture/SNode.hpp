#pragma once

template<typename T>
class SNode
{
public:
	SNode(T data = nullptr);
	SNode(T data, SNode<T>* next);

public:
	T data;
	SNode<T>* next;
};

template<typename T>
inline SNode<T>::SNode(T data) :
	data{ data },
	next{ nullptr }
{
}

template<typename T>
inline SNode<T>::SNode(T data, SNode<T>* next) :
	data{ data },
	next{ next }
{
}
