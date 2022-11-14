#pragma once

#include "pch.h"
#include "StampPtr.hpp"

template<typename T>
class QNode
{
public:
	QNode();
	QNode(T data, QNode<T>* next = nullptr);

public:
	T data;
	StampPtr<T> next;
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
