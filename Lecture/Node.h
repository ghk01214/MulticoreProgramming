#pragma once

#include "pch.h"
#include "MarkableReference.h"

template<typename T>
class Node
{
public:
	Node();
	Node(T data);
	Node(T data, Node<T>* next);

public:
	T data;
	MarkableReference<T> next;
};

template<typename T>
inline Node<T>::Node() :
	next{ false, nullptr }
{
}

template<typename T>
inline Node<T>::Node(T data) :
	data{ data },
	next{ false, nullptr }
{
}

template<typename T>
inline Node<T>::Node(T data, Node<T>* next) :
	data{ data },
	next{ false, next }
{
}
