#pragma once

template<typename T>
class QNode;

template<typename T>
class alignas(16) StampPtr
{
public:
	StampPtr();
	StampPtr(QNode<T>* ptr, int64_t stamp = 0);

	void release();

	QNode<T>* get() { return ptr; }
	void set(QNode<T>* next);

public:
	QNode<T>* ptr;
	int64_t stamp;
};

template<typename T>
inline StampPtr<T>::StampPtr() :
	ptr{ nullptr },
	stamp{ 0 }
{
}

template<typename T>
inline StampPtr<T>::StampPtr(QNode<T>* ptr, int64_t stamp) :
	ptr{ ptr },
	stamp{ stamp }
{
}

template<typename T>
inline void StampPtr<T>::release()
{
	delete ptr;
	stamp = 0;
}

template<typename T>
inline void StampPtr<T>::set(QNode<T>* next)
{
	ptr = next;
	++stamp;
}
