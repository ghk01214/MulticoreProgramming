#include "pch.h"
#include "Bakery.h"

Bakery::Bakery(int32_t num) :
	_flag(num, false),
	_label(num, 0),
	_max{ -1 }
{
	if (_label.empty() == false)
		_max = _label[0];
}

Bakery::~Bakery()
{
}

void Bakery::AddNum(int32_t num)
{
	_flag.resize(num, false);
	_label.resize(num, 0);
	_max = _label[0];
}

void Bakery::Lock(int32_t thread_id)
{
	_flag[thread_id] = true;
	_label[thread_id] = _max + 1;

	for (int32_t i = 0; i < _flag.size(); ++i)
	{
		while ((i != thread_id) && ((_flag[i] == true) && ((_label[i] <= _label[thread_id]) && (i <= thread_id))) {}
	}
}

void Bakery::Unlock(int32_t thread_id)
{
	_flag[thread_id] = false;
}

void Bakery::Max()
{
	for (auto& label : _label)
	{
		if (label > _max)
			_max = label;
	}
}

void Bakery::Clear()
{
	_flag.clear();
	_label.clear();
	_max = -1;
}
