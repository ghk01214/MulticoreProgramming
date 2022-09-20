#include "pch.h"

constexpr int32_t SIZE{ 50000000 };
volatile int32_t x, y;

int32_t trace_x[SIZE];
int32_t trace_y[SIZE];

std::mutex xl, yl;

void Thread1()
{
	for (int32_t i = 0; i < SIZE; ++i)
	{
		xl.lock();
		x = i;
		xl.unlock();
		//std::atomic_thread_fence(std::memory_order_seq_cst);
		yl.lock();
		trace_y[i] = y;
		yl.unlock();
	}
}

void Thread2()
{
	for (int32_t i = 0; i < SIZE; ++i)
	{
		yl.lock();
		y = i;
		yl.unlock();
		//std::atomic_thread_fence(std::memory_order_seq_cst);
		xl.lock();
		trace_x[i] = x;
		xl.unlock();
	}
}

int main()
{
	std::thread t1{ Thread1 };
	std::thread t2{ Thread2 };

	t1.join();
	t2.join();

	int32_t count{ 0 };

	for (int32_t i = 0; i < SIZE; ++i)
	{
		if (trace_x[i] == trace_x[i + 1])
		{
			if (trace_y[trace_x[i]] == trace_y[trace_x[i] + 1])
			{
				if (trace_y[trace_x[i]] != i)
					continue;

				++count;
			}
		}
	}

	std::cout << std::format("Total Memory Inconsistency : {}", count) << std::endl;
}