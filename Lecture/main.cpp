#include "pch.h"

volatile int32_t sum[8 * 8];
std::mutex mu_lock;

void ThreadSum(int32_t id, int32_t thread_num)
{
	for (int i = 0; i < 50000000 / thread_num; ++i)
	{
		sum[id * 8] += 2;
	}
}

int main()
{
	for (int32_t num_thread = 1; num_thread < 9; num_thread *= 2)
	{
		for (auto& n : sum)
		{
			n = 0;
		}
		std::vector<std::thread> threads;

		auto start{ steady_clock::now() };

		for (int32_t i = 0; i < num_thread; ++i)
		{
			threads.emplace_back(ThreadSum, i, num_thread);
		}

		for (auto& thread : threads)
		{
			thread.join();
		}

		auto end{ steady_clock::now() };
		int32_t total{};

		for (auto n : sum)
		{
			total += n;
		}

		//std::cout << std::format("{} threads, sum = {}, delta time = {}\n", num_thread, sum, duration_cast<milliseconds>(end - start).count());
		std::cout << num_thread << " threads, "
			<< "sum = " << total << ", "
			<< "delta time = " << duration_cast<milliseconds>(end - start).count() << std::endl;
	}
}