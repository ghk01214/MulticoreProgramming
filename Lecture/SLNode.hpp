#pragma once

#include "SLMarkablePtr.hpp"

inline constexpr int32_t MAX_LEVEL{ 10 };

template<typename T>
class SLNode
{
public:
	SLNode();
	SLNode(T data, int32_t top_level);

public:
	T data;
	int32_t top_level;
	SLMarkablePtr<T> next[MAX_LEVEL + 1];
};

template<typename T>
inline SLNode<T>::SLNode() :
	top_level{ 0 }
{
}

template<typename T>
inline SLNode<T>::SLNode(T data, int32_t top_level) :
	data{ data },
	top_level{ top_level }
{
}
