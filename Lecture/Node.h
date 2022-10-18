#pragma once

#include "pch.h"
#include "MarkRef.h"

template<typename T>
class Node
{
public:
	Node();
	Node(T data);
	Node(T data, Node<T>* next);

public:
	T data;
	uint32_t retired_epoch;
	MarkRef<T> next;
};

template<typename T>
inline Node<T>::Node() :
	next{ nullptr, false }
{
}

template<typename T>
inline Node<T>::Node(T data) :
	data{ data },
	next{ nullptr, false }
{
}

template<typename T>
inline Node<T>::Node(T data, Node<T>* next) :
	data{ data },
	next{ next, false }
{
}
