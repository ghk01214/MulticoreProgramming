#pragma once

template<typename T>
class QNode;

template<typename T>
class alignas(8) StampPtr
{
public:
	StampPtr(QNode<T>* ptr = nullptr, int64_t stamp = 0);

	QNode<T>* get() { return ptr; }
	void set(QNode<T>* ptr);

public:
	QNode<T>* ptr;
	int64_t stamp;
};

template<typename T>
inline StampPtr<T>::StampPtr(QNode<T>* ptr, int64_t stamp) :
	ptr{ ptr },
	stamp{ stamp }
{
}

template<typename T>
inline void StampPtr<T>::set(QNode<T>* next)
{
	ptr = next;
	++stamp;
}
