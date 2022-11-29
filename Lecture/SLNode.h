#pragma once

inline constexpr int32_t MAX_LEVEL{ 10 };

template<typename T>
class SLNode
{
public:
	SLNode();
	SLNode(T data, int32_t top_level);

	void lock() { lock.lock(); }
	void unlock() { lock.unlock(); }

public:
	T data;
	int32_t top_level;
	SLNode<T>* volatile next[MAX_LEVEL + 1];
	std::recursive_mutex lock;
	volatile bool removed;
	volatile bool fully_linked;
};

template<typename T>
inline SLNode<T>::SLNode() :
	top_level{ 0 },
	removed{ false },
	fully_linked{ false }
{
	for (auto& node : next)
	{
		node = nullptr;
	}
}

template<typename T>
inline SLNode<T>::SLNode(T data, int32_t top_level) :
	data{ data },
	top_level{ top_level },
	removed{ false },
	fully_linked{ false }
{
	for (auto& node : next)
	{
		node = nullptr;
	}
}
