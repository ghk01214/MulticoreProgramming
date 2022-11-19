#pragma once

template<typename T>
class StampPtr;

template<typename T>
class QNode
{
public:
	QNode();
	QNode(T data);
	QNode(T data, StampPtr<T> next);

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
inline QNode<T>::QNode(T data) :
	data{ data },
	next{ nullptr }
{
}

template<typename T>
inline QNode<T>::QNode(T data, StampPtr<T> next) :
	data{ data },
	next{ next }
{
}
