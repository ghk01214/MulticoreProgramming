#pragma once

class Bakery
{
public:
	Bakery() = default;
	Bakery(int32_t num);
	~Bakery();

	void AddNum(int32_t num);
	void Lock(int32_t thread_id);
	void Unlock(int32_t thread_id);
	int32_t Max();
	void Clear();

private:
	std::vector<bool> _flag;
	std::vector<int32_t> _label;
};
