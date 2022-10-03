#include "pch.h"
#include "List.h"

enum : int32_t
{
	NONE = 0,
	LOCK,
	MAX
};

volatile int32_t sum{ 0 };
volatile int32_t lock_memory{ NONE };

std::default_random_engine dre{ std::random_device{}() };
std::uniform_int_distribution<int32_t> uid{ 0, 999 };
std::uniform_int_distribution<int32_t> uid_op{ 0, 2 };

int main()
{
	List<int32_t> list;

	auto start{ steady_clock::now() };
	for (int32_t i = 0; i < 5000000; ++i)
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

	list.Print();
	auto end{ steady_clock::now() };

	std::cout << std::format("Time : {}\n", duration_cast<milliseconds>(end - start));
}