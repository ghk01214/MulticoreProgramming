#include "pch.h"
#include "List.h"

List<int32_t> list;

std::default_random_engine dre{ std::random_device{}() };
std::uniform_int_distribution<int32_t> uid{ 0, 999 };
std::uniform_int_distribution<int32_t> uid_op{ 0, 2 };

void Thread(int32_t num_thread)
{
	for (int32_t i = 0; i < 4000000 / num_thread; ++i)
	{
		switch (uid_op(dre))
		{
			case 0:
			{
				list.insert(uid(dre));
			}
			break;
			case 1:
			{
				list.remove(uid(dre));
			}
			break;
			case 2:
			{
				list.contains(uid(dre));
			}
			break;
		}
	}
}

int main()
{
	for (int32_t thread_num = 1; thread_num <= 8; thread_num *= 2)
	{
		auto start{ steady_clock::now() };

		std::vector<std::thread> threads;
		list.clear();

		for (int32_t i = 0; i < thread_num; ++i)
		{
			threads.emplace_back(Thread, thread_num);
		}

		for (auto& thread : threads)
		{
			thread.join();
		}

		auto end{ steady_clock::now() };

		list.Print();
		std::cout << std::format("Thread Number : {}, Time : {}\n", thread_num, duration_cast<milliseconds>(end - start));
	}
}