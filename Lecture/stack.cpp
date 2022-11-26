#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <random>

using namespace std;
using namespace chrono;

#if _WIN64
using int64 = int64_t;
#else
using int64 = int32_t;
#endif

constexpr int MAX_THREADS = 16;

class null_mutex
{
public:
	void lock() {}
	void unlock() {}
};

class NODE
{
public:
	NODE() : v(-1), next(nullptr) {}
	NODE(int x) : v(x), next(nullptr) {}

public:
	int v;
	NODE* next;
};

class C_STACK
{
public:
	C_STACK() : top (nullptr) {}

	void Push(int x)
	{
		NODE* e = new NODE{ x };
		ll.lock();
		e->next = top;
		top = e;
		ll.unlock();
	}

	int Pop()
	{
		ll.lock();
		if (top == nullptr)
		{
			ll.unlock();
			return -2;
		}
		int res = top->v;
		NODE* t = top;
		top = top->next;
		ll.unlock();
		delete t;
		return res;
	}

	void print20()
	{
		NODE* p = top;
		for (int i = 0; i < 20; ++i)
		{
			if (p == nullptr)
				break;

			cout << p->v << ", ";
			p = p->next;
		}

		cout << endl;
	}

	void clear()
	{
		while (nullptr != top)
		{
			NODE* t = top;
			top = top->next;
			delete t;
		}
	}

private:
	NODE* top;
	mutex ll;
};

class LF_STACK
{
public:
	LF_STACK() : top(nullptr) {}

	bool cas(NODE* volatile* next, NODE* old_p, NODE* new_p)
	{
		return atomic_compare_exchange_strong(
#if _WIN64
			reinterpret_cast<volatile atomic_int64_t*>(next),
#else
			reinterpret_cast<volatile atomic_int32_t*>(next),
#endif
			reinterpret_cast<int64*>(&old_p),
			reinterpret_cast<int64>(new_p));
	}

	void Push(int x)
	{
		NODE* e = new NODE{ x };

		while (true)
		{
			NODE* last = top;
			e->next = last;

			if (true == cas(&top, last, e))
				return;
		}
	}

	int Pop()
	{
		while (true)
		{
			NODE* last = top;

			if (last == nullptr)
				return -2;

			NODE* next = last->next;
			int res = last->v;

			if (true == cas(&top, last, next))
				return res;
		}
	}

	void print20()
	{
		NODE* p = top;

		for (int i = 0; i < 20; ++i)
		{
			if (p == nullptr)
				break;

			cout << p->v << ", ";
			p = p->next;
		}

		cout << endl;
	}

	void clear()
	{
		while (nullptr != top)
		{
			NODE* t = top;
			top = top->next;

			delete t;
		}
	}
	
private:
	NODE* volatile top;
};

#ifndef _WIN64
class BACKOFF
{
public:
	BACKOFF() : limit(10) {}

	void backoff()
	{
		int delay = rand() % limit + 1;
		limit = limit * 2;

		if (limit > 200)
			limit = 200;

		_asm mov eax, delay;
	bo_loop:
		_asm dec eax;
		_asm jnz bo_loop;
	}

private:
	int limit;
};

class LF_BO_STACK
{
public:
	LF_BO_STACK() : top(nullptr) {}

	bool CAS(NODE* volatile* next, NODE* old_p, NODE* new_p)
	{
		return atomic_compare_exchange_strong(
			reinterpret_cast<volatile atomic_long*>(next),
			reinterpret_cast<long*>(&old_p),
			reinterpret_cast<long>(new_p));
	}

	void Push(int x)
	{
		NODE* e = new NODE{ x };
		BACKOFF bo;

		while (true)
		{
			NODE* last = top;
			e->next = last;

			if (true == CAS(&top, last, e))
				return;

			bo.backoff();
		}
	}

	int Pop()
	{
		BACKOFF bo;

		while (true)
		{
			NODE* last = top;

			if (last == nullptr)
				return -2;

			NODE* next = last->next;
			int res = last->v;

			if (true == CAS(&top, last, next))
				return res;

			bo.backoff();
		}
	}

	void print20()
	{
		NODE* p = top;

		for (int i = 0; i < 20; ++i)
		{
			if (p == nullptr)
				break;

			cout << p->v << ", ";
			p = p->next;
		}

		cout << endl;
	}

	void clear()
	{
		while (nullptr != top)
		{
			NODE* t = top;
			top = top->next;

			delete t;
		}
	}

private:
	NODE* volatile top;
};
#endif

constexpr int64 EMPTY = 00;
constexpr int64 WAITING = 01;
constexpr int64 BUSY = 02;

constexpr int TIME_OUT = 1000;

default_random_engine dre{ random_device{}() };
uniform_int_distribution<int32_t> uid{ 1, MAX_THREADS };

class EXCHANGER
{
public:
	EXCHANGER() : value(0) {}
private:
#if _WIN64
	bool cas(atomic_int64_t* value, int64* current_value, int64 next)
#else
	bool cas(atomic_int32_t* value, int64* current_value, int64 next)
#endif
	{
		return atomic_compare_exchange_strong(value, current_value, next);
	}
public:
	int64 exchange(int64 x, bool* is_busy)
	{
#if _WIN64
		int32_t bit = 62;
#else
		int32_t bit = 30;
#endif
		for (int j = 0; j < TIME_OUT; ++j)
		{
			int64 cur_value = value;
			int64 state = value >> bit;

			switch (state)
			{
			case EMPTY:
			{
				int64 new_value = (WAITING << bit) | x;

				if (true == cas(&value, &cur_value, new_value))
				{
					bool success = false;

					for (int i = 0; i < TIME_OUT; ++i)
					{
						if (BUSY == (value >> bit))
						{
							success = true;
							break;
						}
					}

					if (success)
					{
#if _WIN64
						int64 ret = value & 0x3FFFFFFFFFFFFFFF;
#else
						int64 ret = value & 0x3FFFFFFF;
#endif
						value = 0;

						return ret;
					}

					else
					{
						if (true == cas(&value, &new_value, 0))
							return -1;
						else
						{
#if _WIN64
							int64 ret = value & 0x3FFFFFFFFFFFFFFF;
#else
							int64 ret = value & 0x3FFFFFFF;
#endif
							value = 0;

							return ret;
						}
					}
				}
				else
					continue;
			}
			break;
			case WAITING:
			{
				int64 new_value = (BUSY << bit) | x;

				if (true == cas(&value, &cur_value, new_value))
#if _WIN64
					return cur_value & 0x3FFFFFFFFFFFFFFF;
#else
					return cur_value & 0x3FFFFFFF;
#endif
				else
					continue;
			}
			break;
			case BUSY:
				*is_busy = true;
				continue;
				break;
			}
		}

		*is_busy = true;
		return -2;
	}

private:
#if _WIN64
	atomic_int64_t value;		// MSB 2 bit,  00:EMPTY, 01:WAITING, 10:BUSY
#else
	atomic_int32_t value;
#endif
};

class EL_ARRAY
{
public:
	EL_ARRAY() : range(1) {}

	int64 visit(int64 x)
	{
		int slot = uid(dre) - 1;
		bool busy = false;
		int64 ret = ex_array[slot].exchange(x, &busy);
		int64 old_range = range;

		if ((-2 == ret) && (range > 1))
			atomic_compare_exchange_strong(&range, &old_range, old_range - 1);

		if ((true == busy) && (range < MAX_THREADS / 2))
			atomic_compare_exchange_strong(&range, &old_range, old_range + 1);

		return ret;
	}

private:
#if _WIN64
	atomic_int64_t range;
#else
	atomic_int32_t range;
#endif
	EXCHANGER ex_array[MAX_THREADS];
};

class LF_EL_STACK
{
public:
	LF_EL_STACK() : top(nullptr) {}

	bool cas(NODE* volatile* next, NODE* old_p, NODE* new_p)
	{
		return atomic_compare_exchange_strong(
#if _WIN64
			reinterpret_cast<volatile atomic_int64_t*>(next),
#else
			reinterpret_cast<volatile atomic_int32_t*>(next),
#endif
			reinterpret_cast<int64*>(&old_p),
			reinterpret_cast<int64>(new_p));
	}

	void Push(int x)
	{
		NODE* e = new NODE{ x };

		while (true)
		{
			NODE* last = top;
			e->next = last;

			if (cas(&top, last, e) == true)
				return;

			int64 ret = el_.visit(x);

			if (ret == -1)
				return;
		}
	}

	int Pop()
	{
		while (true)
		{
			NODE* last = top;

			if (last == nullptr)
				return -1;

			NODE* next = last->next;
			int32_t res = next->v;

			if (last != top)
				continue;

			if (cas(&top, last, next) == true)
				return res;

			int64 ret = el_.visit(-1);

			if (ret != -1)
				return ret;
		}
	}

	void print20()
	{
		NODE* p = top;

		for (int i = 0; i < 20; ++i)
		{
			if (p == nullptr)
				break;

			cout << p->v << ", ";
			p = p->next;
		}

		cout << endl;
	}

	void clear()
	{
		while (nullptr != top)
		{
			NODE* t = top;
			top = top->next;

			delete t;
		}
	}

private:
	NODE* volatile top;
	EL_ARRAY el_;
};

C_STACK my_stack;
//LF_STACK my_stack;
//LF_BO_STACK my_stack;
//LF_EL_STACK my_stack;
constexpr int NUM_TEST = 10000000;

void worker(int threadNum)
{
	for (int i = 1; i < NUM_TEST / threadNum; i++)
	{
		if ((rand() % 2) || (i < 1000 / threadNum))
			my_stack.Push(i);
		else
			my_stack.Pop();
	}
}

int main()
{
	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) 
	{
		vector <thread> threads;
		my_stack.clear();

		auto start_t = high_resolution_clock::now();

		for (int i = 0; i < num_threads; ++i)
		{
			threads.emplace_back(worker, num_threads);
		}

		for (auto& th : threads)
		{
			th.join();
		}

		auto end_t = high_resolution_clock::now();
		my_stack.print20();

		cout << num_threads << " Threads.  Exec Time : " << duration_cast<milliseconds>(end_t - start_t).count() << endl << endl;
	}
}