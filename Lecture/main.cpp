#include "pch.h"

volatile int32_t victim{ 0 };
volatile bool flag[2]{ false, false };

volatile int32_t sum{ 0 };

void Lock(int32_t th_id)
{
	int32_t other{ 1 - th_id };
	flag[th_id] = true;
	victim = th_id;
	while(flag[other] == true && victim == th_id) {}
}

void Unlock(int32_t th_id)
{
	flag[th_id] = false;
}

void Thread(int32_t th_id)
{
	for (int32_t i = 0; i < 25000000; ++i)
	{
		Lock(th_id);
		sum += 2;
		Unlock(th_id);
	}
}

int main()
{
	auto start{ std::chrono::steady_clock::now() };
	std::thread t1{ Thread, 0 };
	std::thread t2{ Thread, 1 };

	t1.join();
	t2.join();
	auto end{ std::chrono::steady_clock::now() };

	std::cout << "Result : " << sum << ", Time : "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
}