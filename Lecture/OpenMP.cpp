#include <iostream>
#include <format>

#include <omp.h>

int main()
{
	volatile int32_t sum{};

#pragma omp parallel
	{
		for (int32_t i = 0; i < 50000000 / omp_get_num_threads(); ++i)
		{
#pragma omp critical
			sum += 2;
		}
	}

	std::cout << sum << std::endl;
}