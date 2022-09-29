#include <iostream>
#include <format>
#include <chrono>

#include <vector>

#include <thread>
#include <mutex>
#include <atomic>

#include "Bakery.h"

using namespace std::chrono;

void LockFree(int32_t thread_num);
void Mutex(int32_t thread_num);
void Atomic(int32_t thread_num);
void BakeryLock(int32_t thread_num, int32_t thread_id);

volatile int32_t sum{ 0 };
std::atomic_int32_t atomic_sum{ 0 };
std::mutex ml{};
Bakery bl{};

int main()
{
	std::vector<std::thread> threads;

	for (int32_t thread_num = 1; thread_num <= 8; thread_num *= 2)
	{
		auto start{ steady_clock::now() };
		sum = 0;
		atomic_sum = 0;
		threads.clear();
		bl.Clear();

		bl.AddNum(thread_num);

		for (int32_t i = 0; i < thread_num; ++i)
		{
			//threads.emplace_back(LockFree, thread_num);
			//threads.emplace_back(Mutex, thread_num);
			//threads.emplace_back(Atomic, thread_num);
			threads.emplace_back(BakeryLock, thread_num, i);
		}

		for (auto& thread : threads)
		{
			thread.join();
		}

		auto end{ steady_clock::now() };

		std::cout << std::format("Thread Number : {}, Sum = ", thread_num);
		std::cout << sum;
		//std::cout << atomic_sum;
		std::cout << std::format(", Time : {}\n", duration_cast<milliseconds>(end - start));
	}
}

void LockFree(int32_t thread_num)
{
	for (int32_t i = 0; i < 5000000 / thread_num; ++i)
	{
		sum += 2;
	}
}

void Mutex(int32_t thread_num)
{
	for (int32_t i = 0; i < 5000000 / thread_num; ++i)
	{
		ml.lock();
		sum += 2;
		ml.unlock();
	}
}

void Atomic(int32_t thread_num)
{
	for (int32_t i = 0; i < 5000000 / thread_num; ++i)
	{
		atomic_sum += 2;
	}
}

void BakeryLock(int32_t thread_num, int32_t thread_id)
{
	for (int32_t i = 0; i < 5000000 / thread_num; ++i)
	{
		bl.Lock(thread_id);
		sum += 2;
		bl.Unlock(thread_id);
	}
}