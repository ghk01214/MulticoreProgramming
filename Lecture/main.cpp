#include "pch.h"

int32_t error;
volatile int32_t* bound;
volatile bool finish{ false };

void Work()
{
	for (int32_t i = 0; i < 25000000; ++i)
	{
		*bound = -(1 + *bound);
	}

	finish = true;
}

void Check()
{
	while (finish == false)
	{
		int32_t v{ *bound };

		if ((v != 0) && (v != -1))
		{
			std::cout << std::format("{:#x}, ", v);
			++error;
		}
	}
}

int main()
{
	int32_t a[32];
	int64_t address{ reinterpret_cast<int64_t>(&a[30]) };
	
	address = (address / 64) * 64;
	address -= 1;

	bound = reinterpret_cast<int32_t*>(address);
	*bound = 0;

	std::thread t1{ Work };
	std::thread t2{ Check };

	t1.join();
	t2.join();

	std::cout << std::format("\nNumber of error : {}", error) << std::endl;
}