#pragma once

class Bakery
{
public:
	Bakery() = default;
	Bakery(int32_t num);
	~Bakery();

	void Lock();
	void Unlock();

private:
	std::vector<bool> _flag;
	std::vector<int32_t> _label;
};
