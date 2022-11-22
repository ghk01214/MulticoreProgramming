#pragma once

template<typename T>
class Exchanger
{
public:
	Exchanger();
	~Exchanger();

	int32_t exchange(T data, bool* busy);

private:
	int32_t EMPTY;
	int32_t WAITING;
	int32_t BUSY;
	int32_t TIME_OUT;

	int32_t _value;		// MSB 2 bit, 00 : EMPTY, 01 : WAITING, 02 : BUSY
};

template<typename T>
inline Exchanger<T>::Exchanger() :
	EMPTY{ 0 },
	WAITING{ 1 },
	BUSY{ 2 },
	TIME_OUT{ 1000 },
	_value{ 0 }
{
}

template<typename T>
inline Exchanger<T>::~Exchanger()
{
}

template<typename T>
inline int32_t Exchanger<T>::exchange(T data, bool* busy)
{
	for (int32_t j = 0; j < TIME_OUT; ++j)
	{
		int32_t current_value{ data };
		int32_t state{ _value >> 30 };

		switch (state)
		{
			case 0:
			{
				int32_t new_value{ (WAITING << 30) | data };

				if (std::atomic_compare_exchange_strong(&_value, &current_value, new_value) == false)
					break;

				bool success{ false };

				for (int32_t i = 0; i < TIME_OUT; ++i)
				{
					if (BUSY == (_value >> 30))
					{
						success = true;
						break;
					}
				}

				if (success == true)
				{
					int32_t ret{ _value & 0x3FFFFFFF };
					_value = 0;

					return ret;
				}

				if (std::atomic_compare_exchange_strong(&_value, &new_value, 0) == true)
					return -1;

				int32_t ret{ _value & 0x3FFFFFFF };
				_value = 0;

				return ret;

			}
			break;
			case 1:
			{
				int32_t new_value{ (BUSY << 30) | data };

				if (std::atomic_compare_exchange_strong(&_value, &current_value, new_value) == true)
					return current_value & 0x3FFFFFFF;
			}
			break;
			case 2:
			{
				*busy = true;
			}
			break;
		}
	}
	
	*busy = true;

	return -2;
}
