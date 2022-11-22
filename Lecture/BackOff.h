#pragma once

class BackOff
{
public:
	BackOff();
	BackOff(int32_t min, int32_t max);
	~BackOff();

	void InterruptedException();

private:
	struct Delay
	{
		int32_t min;
		int32_t max;
	} _delay;
	int32_t _limit;
};
