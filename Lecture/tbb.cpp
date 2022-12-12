#include "pch.h"
#include <tbb/parallel_for.h>

int32_t main()
{
	std::atomic_int32_t sum{};

	auto start{ std::chrono::steady_clock::now() };
	tbb::parallel_for(0, 50000000, [&sum](int32_t i){ sum += 2; });
	auto end{ std::chrono::steady_clock::now() };

	std::cout << "sum : " << sum << std::endl;
	std::cout << "time : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start) << std::endl;

	sum = 0;
	start = std::chrono::steady_clock::now();

	for (int32_t i = 0; i < 50000000; ++i)
	{
		sum += 2;
	}

	end = std::chrono::steady_clock::now();

	std::cout << "sum : " << sum << std::endl;
	std::cout << "time : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start) << std::endl;
}