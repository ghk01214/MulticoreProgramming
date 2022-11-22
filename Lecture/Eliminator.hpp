#pragma once

#include "Exchanger.hpp"

extern std::default_random_engine dre;

template<typename T>
class Eliminator
{
public:
	Eliminator();
	~Eliminator();

	int32_t visit(T data);

private:
	int32_t _range;
	Exchanger<T> _exchanger[MAX_THREAD];
	std::uniform_int_distribution<int32_t> _uid_range;
};

template<typename T>
inline Eliminator<T>::Eliminator() :
	_range{ 1 },
	_uid_range{ 0, _range - 1 }
{
}

template<typename T>
inline Eliminator<T>::~Eliminator()
{
}

template<typename T>
inline int32_t Eliminator<T>::visit(T data)
{
	int32_t slot{ _uid_range(dre) };
	bool busy{ false };

	int32_t ret{ _exchanger[slot].exchange(data, &busy) };
	int32_t old_range{ _range };

	if (ret == -2 and _range > 1)
		std::atomic_compare_exchange_strong(&_range, &old_range, old_range - 1);

	if (busy == true and _range < MAX_THREAD / 2)
		std::atomic_compare_exchange_strong(&_range, &old_range, old_range + 1);

	return ret;
}
