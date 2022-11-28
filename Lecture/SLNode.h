#pragma once

inline constexpr int32_t MAX_LEVEL{ 10 };

template<typename T>
class SLNode
{
public:
	SLNode();
	SLNode(T data, int32_t top);

public:
	T data;
	int32_t top_level;
	SLNode<T>* volatile next[MAX_LEVEL + 1];
};

template<typename T>
inline SLNode<T>::SLNode() :
	top_level{ 0 }
{
	for (auto& node : next)
	{
		node = nullptr;
	}
}

template<typename T>
inline SLNode<T>::SLNode(T data, int32_t top) :
	data{ data },
	top_level{ top }
{
	for (auto& node : next)
	{
		node = nullptr;
	}
}
