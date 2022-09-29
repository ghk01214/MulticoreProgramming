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

void Bakery::AddNum(int32_t num)
{
	_flag.resize(num, false);
	_label.resize(num, 0);
}

void Bakery::Lock(int32_t thread_id)
{
	_flag[thread_id] = true;
	_label[thread_id] = Max() + 1;
	_flag[thread_id] = false;

	for (int32_t i = 0; i < _flag.size(); ++i)
	{
	//	while ((i != thread_id) && ((_flag[i] == true) && ((_label[i] < _label[thread_id]) || ((_label[i] == _label[thread_id]) && (i <= thread_id))))) {}
		while (_flag[i] == true) {}

		while (_label[i] != 0 &&
			(_label[i] < _label[thread_id] ||
			(_label[i] == _label[thread_id] && i < thread_id))) {}
	}
}

void Bakery::Unlock(int32_t thread_id)
{
	_label[thread_id] = 0;
}

int32_t Bakery::Max()
{
	int32_t max{ _label[0] };

	for (auto& label : _label)
	{
		if (label > max)
			max = label;
	}

	return max;
}

void Bakery::Clear()
{
	_flag.clear();
	_label.clear();
}
