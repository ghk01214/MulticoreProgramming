#pragma once

template<typename T>
class Exchanger
{
public:
	Exchanger();
	~Exchanger();

	int64_t exchange(T data, bool* busy);
private:
	bool cas(T* value, T* old_value, T new_value);

private:
	int64_t EMPTY;
	int64_t WAITING;
	int64_t BUSY;
	int64_t TIME_OUT;

	T _value;		// MSB 2 bit, 00 : EMPTY, 01 : WAITING, 02 : BUSY
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
inline int64_t Exchanger<T>::exchange(T data, bool* busy)
{
	for (int32_t j = 0; j < TIME_OUT; ++j)
	{
		T current_value{ data };
		int64_t state{ _value >> 62 };

		switch (state)
		{
			case 0:
			{
				T new_value{ static_cast<T>((WAITING << 62) | data) };

				if (cas(&_value, &current_value, new_value) == false)
					break;

				bool success{ false };

				for (int32_t i = 0; i < TIME_OUT; ++i)
				{
					if (BUSY == (_value >> 62))
					{
						success = true;
						break;
					}
				}

				if (success == true)
				{
					int64_t ret{ _value & 0x3FFFFFFFFFFFFFFF };
					_value = 0;

					return ret;
				}

				if (cas(&_value, &new_value, 0) == true)
					return -1;

				int64_t ret{ _value & 0x3FFFFFFFFFFFFFFF };
				_value = 0;

				return ret;

			}
			break;
			case 1:
			{
				T new_value{ static_cast<T>((BUSY << 62) | data) };

				if (cas(&_value, &current_value, new_value) == true)
					return current_value & 0x3FFFFFFFFFFFFFFF;
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

template<typename T>
inline bool Exchanger<T>::cas(T* value, T* old_value, T new_value)
{
	return std::atomic_compare_exchange_strong(
		reinterpret_cast<std::atomic_int64_t*>(value),
		reinterpret_cast<int64_t*>(old_value),
		static_cast<int64_t>(new_value)
	);
}
