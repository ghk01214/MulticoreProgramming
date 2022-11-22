#include "pch.h"
#include "List.hpp"
#include "Queue.hpp"
#include "Stack.hpp"

//List<int32_t> cont;
//Queue<int32_t> cont;
Stack<int32_t> cont;

std::default_random_engine dre{ std::random_device{}() };
std::uniform_int_distribution<int32_t> uid_op{ 0, 1 };

void Thread(int32_t num_thread);

int main()
{
	for (int32_t thread_num = 1; thread_num <= MAX_THREAD; thread_num *= 2)
	{
		std::vector<std::thread> threads;
		cont.clear();

		auto start{ steady_clock::now() };

		for (int32_t i = 0; i < thread_num; ++i)
		{
			threads.emplace_back(Thread, thread_num);
		}

		for (auto& thread : threads)
		{
			thread.join();
		}

		auto end{ steady_clock::now() };

		cont.Print();
		std::cout << std::format("Thread Number : {}, Time : {}\n", thread_num, duration_cast<milliseconds>(end - start)) << std::endl;
	}
}

void Thread(int32_t num_thread)
{
	for (int32_t i = 0; i < 10000000 / num_thread; ++i)
	{
		if (uid_op(dre) == 1 or i < 1000 / num_thread)
			cont.push(i);
		else
			cont.pop();
	}
}