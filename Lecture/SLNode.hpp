#pragma once

template<typename T>
class SLNode
{
public:
	SLNode();
	~SLNode();

public:
	T data;
	SLNode<T>* next;
};

