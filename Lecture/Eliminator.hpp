#pragma once

#include "Exchanger.hpp"

template<typename T>
class Eliminator
{
public:
	Eliminator();
	~Eliminator();

	int64_t visit(T data);
private:
	bool cas(std::atomic_int64_t* range, int64_t* old_range, int64_t new_range);

private:
	std::atomic_int64_t _range;
	Exchanger<T> _exchanger[MAX_THREAD];
	std::uniform_int_distribution<int32_t> _uid_range;
};

template<typename T>
inline Eliminator<T>::Eliminator() :
	_range{ 1 },
	_uid_range{ 1, _range }
{
}

template<typename T>
inline Eliminator<T>::~Eliminator()
{
}

template<typename T>
inline int64_t Eliminator<T>::visit(T data)
{
	extern std::default_random_engine dre;

	int32_t slot{ _uid_range(dre) - 1 };
	bool busy{ false };

	int64_t ret{ _exchanger[slot].exchange(data, &busy) };
	int64_t old_range{ _range.load() };

	if (ret == -2 and _range > 1)
		cas(&_range, &old_range, old_range - 1);

	if (busy == true and _range < MAX_THREAD / 2)
		cas(&_range, &old_range, old_range + 1);

	return ret;
}

template<typename T>
inline bool Eliminator<T>::cas(std::atomic_int64_t* range, int64_t* old_range, int64_t new_range)
{
	return std::atomic_compare_exchange_strong(range, old_range, new_range);
}
