#include <vector>
#include "Bakery.h"

Bakery::Bakery(int32_t num) :
	_flag(num, false),
	_label(num, 0)
{
}

Bakery::~Bakery()
{
}

void Bakery::Lock()
{
}

void Bakery::Unlock()
{
}
