#include "pch.h"

int sum{ 0 };
std::mutex m;

void ThreadSum()
{
	std::lock_guard g{ m };

	for (int i = 0; i < 25000000; ++i)
	{
		sum += 2;
	}
}

int main()
{
	std::thread thread1{ ThreadSum };
	std::thread thread2{ ThreadSum };

	thread1.join();
	thread2.join();

	std::cout << std::format("sum = {}\n", sum);
}