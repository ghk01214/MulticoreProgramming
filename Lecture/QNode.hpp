#pragma once

#include "pch.h"

template<typename T>
class QNode
{
public:
	QNode();
	QNode(T data, QNode<T>* next = nullptr);

public:
	T data;
	QNode<T>* next;
};

template<typename T>
inline QNode<T>::QNode() :
	next{ nullptr }
{
}

template<typename T>
inline QNode<T>::QNode(T data, QNode<T>* next) :
	data{ data },
	next{ next }
{
}
