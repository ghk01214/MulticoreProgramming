#include "pch.h"

enum : int32_t
{
	NONE = 0,
	LOCK,
	MAX
};

volatile int32_t sum{ 0 };
volatile int32_t lock_memory{ NONE };

bool cas(volatile int32_t* address, int32_t expected, int32_t update)
{
	return std::atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic_int32_t*>(address), &expected, update);
}

void Lock()
{
	// lock_memory == LOCK이면 NONE이 될 때까지 대기
	// lock_memory == NONE이면 atomic하게 LOCK로 변환 후 return
	while (cas(&lock_memory, NONE, LOCK) == false) {}
}

void Unlock()
{
	// atomic하게 lock_memory를 NONE으로 변환
	cas(&lock_memory, LOCK, NONE);
}

void Thread(int32_t threads)
{
		Lock();
	for (int32_t i = 0; i < 50000000 / threads; ++i)
	{
		sum += 2;
	}
		Unlock();
}

int main()
{
	for (int32_t thread_num = 1; thread_num <= 8; thread_num *= 2)
	{
		auto start{ steady_clock::now() };
		sum = 0;

		std::vector<std::thread> threads;

		for (int32_t i = 0; i < thread_num; ++i)
		{
			threads.emplace_back(Thread, thread_num);
		}

		for (auto& thread : threads)
		{
			thread.join();
		}

		auto end{ steady_clock::now() };

		std::cout << std::format("Thread Number : {}, Sum = ", thread_num);
		std::cout << sum;
		std::cout << std::format(", Time : {}\n", duration_cast<milliseconds>(end - start));
	}
}