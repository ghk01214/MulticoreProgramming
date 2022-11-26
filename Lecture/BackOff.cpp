#ifndef _WIN64
#include "pch.h"
#include "BackOff.h"

extern std::default_random_engine dre;

BackOff::BackOff() :
	_delay{},
	_limit{ 10 }
{
}

BackOff::BackOff(int32_t min, int32_t max) :
	_delay{ min, max },
	_limit{ min }
{
}

BackOff::~BackOff()
{
}

void BackOff::InterruptedException()
{
	std::uniform_int_distribution<int32_t> uid_limit{ 1, _limit };
	int32_t delay{ uid_limit(dre) };

	_limit *= 2;

	if (_limit > _delay.max)
		_limit = _delay.max;

	_asm mov eax, delay;
loop:
	_asm dec eax;
	_asm jnz loop;
}

#endif