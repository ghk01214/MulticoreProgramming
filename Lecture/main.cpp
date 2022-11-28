#include "pch.h"
#include "List.hpp"
#include "Queue.hpp"
#include "Stack.hpp"
#include "SkipList.hpp"

//List<int32_t> cont;
//Queue<int32_t> cont;
//Stack<int32_t> cont;
SkipList<int32_t> cont;

std::default_random_engine dre{ std::random_device{}() };
std::uniform_int_distribution<int32_t> uid{ 1, 1000 };
std::uniform_int_distribution<int32_t> uid_op{ 0, 2 };

struct History
{
	History(int32_t operation, int32_t input, bool output) : operation{ operation }, input{ input }, output{ output } {}

	int32_t operation;
	int32_t input;
	bool output;
};

void Thread(int32_t num_thread);
void ThreadCheck(std::vector<History>* history, int32_t num_thread);
void CheckHistory(std::array<std::vector<History>, 16>& history, int32_t num_thread);

int main()
{
	std::cout << std::format("=================CONSISTENCY CHECK=================") << std::endl;

	for (int32_t thread_num = 1; thread_num <= 16; thread_num *= 2)
	{
		std::array<std::vector<History>, 16> history;
		std::vector<std::thread> threads;
		cont.clear();

		auto start{ steady_clock::now() };

		for (int32_t i = 0; i < thread_num; ++i)
		{
			threads.emplace_back(ThreadCheck, &history[i], thread_num);
		}

		for (auto& thread : threads)
		{
			thread.join();
		}

		auto end{ steady_clock::now() };

		cont.Print();
		std::cout << std::format("Thread Number : {}, Time : {}\n", thread_num, duration_cast<milliseconds>(end - start));
		CheckHistory(history, thread_num);
	}

	std::cout << std::format("=================SPEED CHECK=================") << std::endl;

	for (int32_t thread_num = 1; thread_num <= 16; thread_num *= 2)
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
	for (int32_t i = 0; i < 4000000 / num_thread; ++i)
	{
		switch (uid_op(dre))
		{
			case 0:
			{
				cont.insert(uid(dre));
			}
			break;
			case 1:
			{
				cont.remove(uid(dre));
			}
			break;
			case 2:
			{
				cont.contains(uid(dre));
			}
			break;
		}
	}
}

void ThreadCheck(std::vector<History>* history, int32_t num_thread)
{
	for (int32_t i = 0; i < 4000000 / num_thread; ++i)
	{
		int32_t value{ uid(dre) };

		switch (uid_op(dre))
		{
			case 0:
			{
				history->emplace_back(0, value, cont.insert(value));
			}
			break;
			case 1:
			{
				history->emplace_back(1, value, cont.remove(value));
			}
			break;
			case 2:
			{
				history->emplace_back(2, value, cont.contains(value));
			}
			break;
		}
	}
}

void CheckHistory(std::array<std::vector<History>, 16>& history, int32_t num_thread)
{
	std::array<int32_t, 1000> survive{};

	std::cout << std::format("Checking Consistency : ");

	if (history[0].empty() == true)
	{
		std::cout << std::format("No history\n");
		return;
	}

	for (int32_t i = 0; i < num_thread; ++i)
	{
		for (auto& op : history[i])
		{
			if (op.output == false)
				continue;

			if (op.operation == 3)
				continue;

			if (op.operation == 0)
				++survive[op.input];

			if (op.operation == 1)
				--survive[op.input];
		}
	}

	for (int32_t i = 0; i < 1000; ++i)
	{
		int32_t value{ survive[i] };

		if (value < 0)
		{
			std::cout << std::format("The value {} is removed while it is not in the list\n", i);
			exit(-1);
		}
		else if (value > 1)
		{
			std::cout << std::format("The value {} is added while the list already have it\n", i);
			exit(-1);
		}
		else if (value == 0)
		{
			if (cont.contains(i) == true)
			{
				std::cout << std::format("The value {} should not exist\n", i);
				exit(-1);
			}
		}
		else if (value == 1)
		{
			if (cont.contains(i) == false)
			{
				std::cout << std::format("The value {} should exist\n", i);
				exit(-1);
			}
		}
	}

	std::cout << std::format(" OK\n") << std::endl;
}