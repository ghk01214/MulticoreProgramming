#include "pch.h"

static const auto NUM_TEST = 4000;
static const auto KEY_RANGE = 1000;
static const auto MAX_THREAD = 64;

thread_local int32_t thread_id;

// 합의 객체
class Consensus {
public:
	Consensus() : result{ -1 } {}
	~Consensus() = default;

	int64_t decide(int64_t value)
	{
		if (cas(-1, value) == true)
			return value;

		return result;
	}

private:
	bool cas(int64_t old_value, int64_t new_value)
	{
		return std::atomic_compare_exchange_strong(reinterpret_cast<std::atomic_int64_t*>(&result), &old_value, new_value);
	}

private:
	int64_t result;
};

typedef bool Response;

enum Method
{
	M_ADD,
	M_REMOVE,
	M_CONTAINS
};

struct Invocation
{
	Method method;
	int32_t	input;
};

class SeqObject
{
public:
	SeqObject() = default;
	~SeqObject() = default;

	Response Apply(const Invocation& invoc)
	{
		Response res;

		switch (invoc.method)
		{
			case M_ADD:
			{
				if (seq_set.count(invoc.input) != 0)
				{
					res = false;
				}
				else
				{
					seq_set.insert(invoc.input);
					res = true;
				}
			}
			break;
			case M_REMOVE:
			{
				if (seq_set.count(invoc.input) == 0)
				{
					res = false;
				}
				else
				{
					seq_set.erase(invoc.input);
					res = true;
				}
			}
			break;
			case M_CONTAINS:
			{
				res = (seq_set.count(invoc.input) != 0);
			}
			break;
		}

		return res;
	}

	void Print20()
	{
		std::cout << std::format("First 20 item : ");

		int32_t count = 20;

		for (auto n : seq_set)
		{
			if (count-- == 0)
				break;

			std::cout << std::format("{}, ", n);
		}

		std::cout << std::endl;

	}

	void clear()
	{
		seq_set.clear();
	}

private:
	std::set<int32_t> seq_set;
};

class Node
{
public:
	Node() : seq{ 0 }, next{ nullptr } {}
	Node(const Invocation& input_invoc) : invoc{ input_invoc }, seq{ 0 }, next{ nullptr } {}
	~Node() = default;

public:
	Invocation invoc;
	Consensus decideNext;
	Node* next;
	volatile int32_t seq;
};

class LFUniversal
{
public:
	LFUniversal() :
		tail{ new Node }
	{
		tail->seq = 1;

		for (int32_t i = 0; i < MAX_THREAD; ++i)
		{
			head[i] = tail;
		}
	}
	~LFUniversal()
	{
		clear();
	}

	Node* GetMaxNODE()
	{
		Node* max_node = head[0];

		for (int32_t i = 1; i < MAX_THREAD; i++)
		{
			if (max_node->seq < head[i]->seq)
				max_node = head[i];
		}

		return max_node;
	}

	void clear()
	{
		while (tail != nullptr)
		{
			Node* temp = tail;
			tail = tail->next;

			delete temp;
		}

		tail = new Node;
		tail->seq = 1;

		for (int32_t i = 0; i < MAX_THREAD; ++i)
		{
			head[i] = tail;
		}
	}

	Response Apply(const Invocation& invoc)
	{
		Node* prefer = new Node{ invoc };

		while (prefer->seq == 0)
		{
			Node* before = GetMaxNODE();
			Node* after = reinterpret_cast<Node*>(before->decideNext.decide(reinterpret_cast<int64_t>(prefer)));

			before->next = after;
			after->seq = before->seq + 1;

			head[thread_id] = after;
		}

		SeqObject my_object;
		Node* curr = tail->next;

		while (curr != prefer)
		{
			my_object.Apply(curr->invoc);
			curr = curr->next;
		}

		return my_object.Apply(curr->invoc);
	}

	void Print20()
	{
		Node* before = GetMaxNODE();

		SeqObject my_object;

		Node* curr = tail->next;

		while (true)
		{
			my_object.Apply(curr->invoc);

			if (curr == before)
				break;

			curr = curr->next;

		}

		my_object.Print20();
	}

private:
	Node* head[MAX_THREAD];
	Node* tail;
};

class MutexUniversal
{
public:
	MutexUniversal() = default;
	~MutexUniversal() = default;

	void clear()
	{
		seq_object.clear();
	}

	Response Apply(const Invocation& invoc)
	{
		m_lock.lock();
		Response res = seq_object.Apply(invoc);
		m_lock.unlock();

		return res;
	}

	void Print20()
	{
		seq_object.Print20();
	}

private:
	SeqObject seq_object;
	std::mutex m_lock;
};

class WFUniversal
{
public:
	WFUniversal() :
		tail{ new Node }
	{
		tail->seq = 1;

		for (int32_t i = 0; i < MAX_THREAD; ++i)
		{
			head[i] = tail;
			announce[i] = tail;
		}
	}
	~WFUniversal()
	{
		clear();
	}

	Node* GetMaxNODE()
	{
		Node* max_node = head[0];

		for (int32_t i = 1; i < MAX_THREAD; i++)
		{
			if (max_node->seq < head[i]->seq)
				max_node = head[i];
		}

		return max_node;
	}

	void clear()
	{
		while (tail != nullptr)
		{
			Node* temp = tail;
			tail = tail->next;

			delete temp;
		}

		tail = new Node;
		tail->seq = 1;

		for (int32_t i = 0; i < MAX_THREAD; ++i)
		{
			head[i] = tail;
			announce[i] = tail;
		}
	}

	Response Apply(const Invocation& invoc)
	{
		announce[thread_id] = new Node{ invoc };
		head[thread_id] = GetMaxNODE();

		while (announce[thread_id]->seq == 0)
		{
			Node* before = head[thread_id];
			Node* help = announce[(before->seq + 1) % MAX_THREAD];
			Node* prefer;

			if (help->seq == 0)
				prefer = help;
			else
				prefer = announce[thread_id];

			Node* after = reinterpret_cast<Node*>(before->decideNext.decide(reinterpret_cast<int64_t>(prefer)));

			before->next = after;
			after->seq = before->seq + 1;

			head[thread_id] = after;
		}

		SeqObject my_object;
		Node* curr = tail->next;

		while (curr != announce[thread_id])
		{
			my_object.Apply(curr->invoc);
			curr = curr->next;
		}

		head[thread_id] = announce[thread_id];

		return my_object.Apply(curr->invoc);
	}

	void Print20()
	{
		Node* before = GetMaxNODE();

		SeqObject my_object;

		Node* curr = tail->next;

		while (true)
		{
			my_object.Apply(curr->invoc);

			if (curr == before)
				break;

			curr = curr->next;

		}

		my_object.Print20();
	}

private:
	Node* announce[MAX_THREAD];
	Node* head[MAX_THREAD];
	Node* tail;
};

//SeqObject my_set;
//MutexUniversal my_set;
//LFUniversal my_set;
WFUniversal my_set;

void Benchmark(int32_t num_thread, int32_t tid)
{
	thread_id = tid;

	Invocation invoc;

	for (int32_t i = 0; i < NUM_TEST / num_thread; ++i)
	{
		switch (rand() % 3)
		{
			case 0:
				invoc.method = M_ADD;
				break;
			case 1:
				invoc.method = M_REMOVE;
				break;
			case 2:
				invoc.method = M_CONTAINS;
				break;
		}

		invoc.input = rand() % KEY_RANGE;
		my_set.Apply(invoc);
	}
}

int32_t main()
{
	std::vector<std::thread*> worker_threads;
	thread_id = 0;

	for (int32_t num_thread = 1; num_thread <= 16; num_thread *= 2)
	{
		my_set.clear();

		auto start = steady_clock::now();

		for (int32_t i = 0; i < num_thread; ++i)
		{
			worker_threads.push_back(new std::thread{ Benchmark, num_thread, i + 1 });
		}

		for (auto pth : worker_threads)
		{
			pth->join();
		}

		auto du = steady_clock::now() - start;

		for (auto pth : worker_threads)
		{
			delete pth;
		}

		worker_threads.clear();

		std::cout << std::format("{} Threads	", num_thread);
		std::cout << std::format("{}\n", duration_cast<milliseconds>(du));
		my_set.Print20();
	}
}