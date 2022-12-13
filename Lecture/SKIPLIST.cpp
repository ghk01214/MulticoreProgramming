#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <array>
#include <mutex>

using namespace std;
using namespace chrono;

class null_mutex {
public:
	void lock() {}
	void unlock() {}
};

class NODE {
	mutex n_lock;
public:
	int v;
	NODE* volatile next;
	volatile bool removed;
	NODE() : v(-1), next(nullptr), removed(false) {}
	NODE(int x) : v(x), next(nullptr), removed(false) {}
	void lock()
	{
		n_lock.lock();
	}
	void unlock()
	{
		n_lock.unlock();
	}
};

class NODE_SP {
	mutex n_lock;
public:
	int v;
	shared_ptr <NODE_SP> next;
	volatile bool removed;
	NODE_SP() : v(-1), next(nullptr), removed(false) {}
	NODE_SP(int x) : v(x), next(nullptr), removed(false) {}
	void lock()
	{
		n_lock.lock();
	}
	void unlock()
	{
		n_lock.unlock();
	}
};

class LF_NODE;
class LF_PTR {
	unsigned long long next;
public:
	LF_PTR() : next(0) {}
	LF_PTR(bool marking, LF_NODE* ptr)
	{
		next = reinterpret_cast<unsigned long long>(ptr);
		if (true == marking) next = next | 1;
	}
	LF_NODE* get_ptr()
	{
		return reinterpret_cast<LF_NODE*>(next & 0xFFFFFFFFFFFFFFFE);
	}
	bool get_removed()
	{
		return (next & 1) == 1;
	}
	LF_NODE* get_ptr_mark(bool* removed)
	{
		unsigned long long cur_next = next;
		*removed = (cur_next & 1) == 1;
		return reinterpret_cast<LF_NODE*>(cur_next & 0xFFFFFFFFFFFFFFFE);
	}
	bool CAS(LF_NODE* o_ptr, LF_NODE* n_ptr, bool o_mark, bool n_mark)
	{
		unsigned long long o_next = reinterpret_cast<unsigned long long>(o_ptr);
		if (true == o_mark) o_next++;
		unsigned long long n_next = reinterpret_cast<unsigned long long>(n_ptr);
		if (true == n_mark) n_next++;
		return atomic_compare_exchange_strong(
			reinterpret_cast<atomic_uint64_t*>(&next), &o_next, n_next);
	}
	bool is_removed()
	{
		return (next & 1) == 1;
	}
};

class LF_NODE {
public:
	int v;
	LF_PTR next;
	LF_NODE() : v(-1), next(false, nullptr) {}
	LF_NODE(int x) : v(x), next(false, nullptr) {}
	LF_NODE(int x, LF_NODE* ptr) : v(x), next(false, ptr) {}
};

class SET {
	NODE head, tail;
	mutex ll;
public:
	SET()
	{
		head.v = 0x80000000;
		tail.v = 0x7FFFFFFF;
		head.next = &tail;
		tail.next = nullptr;
	}
	bool ADD(int x)
	{
		NODE* prev = &head;
		ll.lock();
		NODE* curr = prev->next;
		while (curr->v < x) {
			prev = curr;
			curr = curr->next;
		}
		if (curr->v != x) {
			NODE* node = new NODE{ x };
			node->next = curr;
			prev->next = node;
			ll.unlock();
			return true;
		}
		else
		{
			ll.unlock();
			return false;
		}
	}

	bool REMOVE(int x)
	{
		NODE* prev = &head;
		ll.lock();
		NODE* curr = prev->next;
		while (curr->v < x) {
			prev = curr;
			curr = curr->next;
		}
		if (curr->v != x) {
			ll.unlock();
			return false;
		}
		else {
			prev->next = curr->next;
			delete curr;
			ll.unlock();
			return true;
		}
	}

	bool CONTAINS(int x)
	{
		NODE* prev = &head;
		ll.lock();
		NODE* curr = prev->next;
		while (curr->v < x) {
			prev = curr;
			curr = curr->next;
		}
		bool res = (curr->v == x);
		ll.unlock();
		return res;
	}
	void print20()
	{
		NODE* p = head.next;
		for (int i = 0; i < 20; ++i) {
			if (p == &tail) break;
			cout << p->v << ", ";
			p = p->next;
		}
		cout << endl;
	}

	void clear()
	{
		NODE* p = head.next;
		while (p != &tail) {
			NODE* t = p;
			p = p->next;
			delete t;
		}
		head.next = &tail;
	}
};

class F_SET {
	NODE head, tail;
public:
	F_SET()
	{
		head.v = 0x80000000;
		tail.v = 0x7FFFFFFF;
		head.next = &tail;
		tail.next = nullptr;
	}
	bool ADD(int x)
	{
		head.lock();
		NODE* prev = &head;
		NODE* curr = prev->next;
		curr->lock();
		while (curr->v < x) {
			prev->unlock();
			prev = curr;
			curr = curr->next;
			curr->lock();
		}
		if (curr->v != x) {
			NODE* node = new NODE{ x };
			node->next = curr;
			prev->next = node;
			curr->unlock();
			prev->unlock();
			return true;
		}
		else
		{
			curr->unlock();
			prev->unlock();
			return false;
		}
	}

	bool REMOVE(int x)
	{
		head.lock();
		NODE* prev = &head;
		NODE* curr = prev->next;
		curr->lock();
		while (curr->v < x) {
			prev->unlock();
			prev = curr;
			curr = curr->next;
			curr->lock();
		}
		if (curr->v != x) {
			curr->unlock();
			prev->unlock();
			return false;
		}
		else {
			prev->next = curr->next;
			curr->unlock();
			prev->unlock();
			delete curr;
			return true;
		}
	}

	bool CONTAINS(int x)
	{
		head.lock();
		NODE* prev = &head;
		NODE* curr = prev->next;
		curr->lock();
		while (curr->v < x) {
			prev->unlock();
			prev = curr;
			curr = curr->next;
			curr->lock();
		}
		bool res = (curr->v == x);
		curr->unlock();
		prev->unlock();
		return res;
	}
	void print20()
	{
		NODE* p = head.next;
		for (int i = 0; i < 20; ++i) {
			if (p == &tail) break;
			cout << p->v << ", ";
			p = p->next;
		}
		cout << endl;
	}

	void clear()
	{
		NODE* p = head.next;
		while (p != &tail) {
			NODE* t = p;
			p = p->next;
			delete t;
		}
		head.next = &tail;
	}
};

class O_SET {
	NODE head, tail;
public:
	O_SET()
	{
		head.v = 0x80000000;
		tail.v = 0x7FFFFFFF;
		head.next = &tail;
		tail.next = nullptr;
	}
	bool validate(NODE* prev, NODE* curr)
	{
		NODE* n = &head;
		while (n->v <= prev->v) {
			if (n == prev)
				return prev->next == curr;
			n = n->next;
		}
		return false;
	}

	bool ADD(int x)
	{
		while (true) {
			NODE* prev = &head;
			NODE* curr = prev->next;
			while (curr->v < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->lock(); curr->lock();
			if (validate(prev, curr)) {
				if (curr->v != x) {
					NODE* node = new NODE{ x };
					node->next = curr;
					prev->next = node;
					curr->unlock();
					prev->unlock();
					return true;
				}
				else
				{
					curr->unlock();
					prev->unlock();
					return false;
				}
			}
			else {
				curr->unlock();
				prev->unlock();
			}
		}
	}

	bool REMOVE(int x)
	{
		while (true) {
			NODE* prev = &head;
			NODE* curr = prev->next;
			while (curr->v < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->lock(); curr->lock();
			if (validate(prev, curr)) {
				if (curr->v != x) {
					curr->unlock();
					prev->unlock();
					return false;
				}
				else
				{
					prev->next = curr->next;
					curr->unlock();
					prev->unlock();
					return true;
				}
			}
			else {
				curr->unlock();
				prev->unlock();
			}
		}
	}

	bool CONTAINS(int x)
	{
		while (true) {
			NODE* prev = &head;
			NODE* curr = prev->next;
			while (curr->v < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->lock(); curr->lock();
			if (validate(prev, curr)) {
				if (curr->v == x) {
					curr->unlock();
					prev->unlock();
					return true;
				}
				else
				{
					curr->unlock();
					prev->unlock();
					return false;
				}
			}
			else {
				curr->unlock();
				prev->unlock();
			}
		}
	}

	void print20()
	{
		NODE* p = head.next;
		for (int i = 0; i < 20; ++i) {
			if (p == &tail) break;
			cout << p->v << ", ";
			p = p->next;
		}
		cout << endl;
	}

	void clear()
	{
		NODE* p = head.next;
		while (p != &tail) {
			NODE* t = p;
			p = p->next;
			delete t;
		}
		head.next = &tail;
	}
};

class L_SET {
	NODE head, tail;
public:
	L_SET()
	{
		head.v = 0x80000000;
		tail.v = 0x7FFFFFFF;
		head.next = &tail;
		tail.next = nullptr;
	}
	bool validate(NODE* prev, NODE* curr)
	{
		return (prev->removed == false) && (curr->removed == false)
			&& (prev->next == curr);
	}

	bool ADD(int x)
	{
		while (true) {
			NODE* prev = &head;
			NODE* curr = prev->next;
			while (curr->v < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->lock(); curr->lock();
			if (validate(prev, curr)) {
				if (curr->v != x) {
					NODE* node = new NODE{ x };
					node->next = curr;
					prev->next = node;
					curr->unlock();
					prev->unlock();
					return true;
				}
				else
				{
					curr->unlock();
					prev->unlock();
					return false;
				}
			}
			else {
				curr->unlock();
				prev->unlock();
			}
		}
	}

	bool REMOVE(int x)
	{
		while (true) {
			NODE* prev = &head;
			NODE* curr = prev->next;
			while (curr->v < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->lock(); curr->lock();
			if (validate(prev, curr)) {
				if (curr->v != x) {
					curr->unlock();
					prev->unlock();
					return false;
				}
				else
				{
					curr->removed = true;
					prev->next = curr->next;
					curr->unlock();
					prev->unlock();
					return true;
				}
			}
			else {
				curr->unlock();
				prev->unlock();
			}
		}
	}

	bool CONTAINS(int x)
	{
		while (true) {
			NODE* prev = &head;
			NODE* curr = prev->next;
			while (curr->v < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->lock(); curr->lock();
			if (validate(prev, curr)) {
				if (curr->v == x) {
					curr->unlock();
					prev->unlock();
					return true;
				}
				else
				{
					curr->unlock();
					prev->unlock();
					return false;
				}
			}
			else {
				curr->unlock();
				prev->unlock();
			}
		}
	}

	void print20()
	{
		NODE* p = head.next;
		for (int i = 0; i < 20; ++i) {
			if (p == &tail) break;
			cout << p->v << ", ";
			p = p->next;
		}
		cout << endl;
	}

	void clear()
	{
		NODE* p = head.next;
		while (p != &tail) {
			NODE* t = p;
			p = p->next;
			delete t;
		}
		head.next = &tail;
	}
};

class LF_SET {
	LF_NODE head, tail;
public:
	LF_SET()
	{
		head.v = 0x80000000;
		tail.v = 0x7FFFFFFF;
		head.next = LF_PTR{ false, &tail };
	}

	void Find(LF_NODE*& prev, LF_NODE*& curr, int x)
	{
		while (true) {
		retry:
			prev = &head;
			curr = prev->next.get_ptr();
			while (true) {
				bool removed;
				LF_NODE* succ = curr->next.get_ptr_mark(&removed);
				while (true == removed) {
					if (false == prev->next.CAS(curr, succ, false, false)) {
						goto retry;
					}
					curr = succ;
					succ = curr->next.get_ptr_mark(&removed);
				}
				if (curr->v >= x) return;
				prev = curr;
				curr = succ;
			}
		}
	}

	bool ADD(int x)
	{
		while (true) {
			LF_NODE* prev, * curr;
			Find(prev, curr, x);
			if (curr->v != x) {
				LF_NODE* node = new LF_NODE{ x, curr };
				if (true == prev->next.CAS(curr, node, false, false))
					return true;
				delete node;
			}
			else
			{
				return false;
			}
		}
	}

	bool REMOVE(int x)
	{
		while (true) {
			LF_NODE* prev, * curr;
			Find(prev, curr, x);
			if (curr->v == x) {
				LF_NODE* succ = curr->next.get_ptr();
				if (false == curr->next.CAS(succ, succ, false, true))
					continue;
				prev->next.CAS(curr, succ, false, false);
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	bool CONTAINS(int x)
	{
		LF_NODE* node = head.next.get_ptr();
		while (node->v < x) node = node->next.get_ptr();
		return ((node->v == x) && (false == node->next.is_removed()));
		return true;
	}

	void print20()
	{
		LF_NODE* p = head.next.get_ptr();
		for (int i = 0; i < 20; ++i) {
			if (p == &tail) break;
			cout << p->v << ", ";
			p = p->next.get_ptr();
		}
		cout << endl;
	}

	void clear()
	{
		LF_NODE* p = head.next.get_ptr();
		while (p != &tail) {
			LF_NODE* t = p;
			p = p->next.get_ptr();
			delete t;
		}
		head.next = LF_PTR{ false, &tail };
	}
};

class L_SET_SP {
	shared_ptr <NODE_SP> head, tail;
public:
	L_SET_SP()
	{
		head = make_shared<NODE_SP>(0x80000000);
		tail = make_shared<NODE_SP>(0x7FFFFFFF);
		head->next = tail;
	}
	bool validate(shared_ptr<NODE_SP>& prev, shared_ptr<NODE_SP>& curr)
	{
		return (prev->removed == false) && (curr->removed == false)
			&& (prev->next == curr);
	}

	bool ADD(int x)
	{
		while (true) {
			shared_ptr<NODE_SP> prev = head;
			shared_ptr<NODE_SP> curr = prev->next;
			while (curr->v < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->lock(); curr->lock();
			if (validate(prev, curr)) {
				if (curr->v != x) {
					shared_ptr<NODE_SP> node = make_shared<NODE_SP>(x);;
					node->next = curr;
					prev->next = node;
					curr->unlock();
					prev->unlock();
					return true;
				}
				else
				{
					curr->unlock();
					prev->unlock();
					return false;
				}
			}
			else {
				curr->unlock();
				prev->unlock();
			}
		}
	}

	bool REMOVE(int x)
	{
		while (true) {
			shared_ptr<NODE_SP> prev = head;
			shared_ptr<NODE_SP> curr = prev->next;
			while (curr->v < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->lock(); curr->lock();
			if (validate(prev, curr)) {
				if (curr->v != x) {
					curr->unlock();
					prev->unlock();
					return false;
				}
				else
				{
					curr->removed = true;
					prev->next = curr->next;
					curr->unlock();
					prev->unlock();
					return true;
				}
			}
			else {
				curr->unlock();
				prev->unlock();
			}
		}
	}

	bool CONTAINS(int x)
	{
		shared_ptr<NODE_SP> node = head->next;
		while (node->v < x) node = node->next;
		return ((node->v == x) && (false == node->removed));
	}

	void print20()
	{
		shared_ptr<NODE_SP> p = head->next;
		for (int i = 0; i < 20; ++i) {
			if (p == tail) break;
			cout << p->v << ", ";
			p = p->next;
		}
		cout << endl;
	}

	void clear()
	{
		head->next = tail;
	}
};


constexpr int MAX_LEVEL = 10;
class NODE_SK {
public:
	int v;
	NODE_SK* volatile next[MAX_LEVEL + 1];
	int	top_level;
	recursive_mutex ll;
	volatile bool removed;
	volatile bool fullylinked;
	NODE_SK() : v(-1), top_level(0), removed(false), fullylinked(false)
	{
		for (auto& n : next) n = nullptr;
	}
	NODE_SK(int x, int top) : v(x), top_level(top), removed(false), fullylinked(false)
	{
		for (auto& n : next) n = nullptr;
	}
	void lock()
	{
		ll.lock();
	}
	void unlock()
	{
		ll.unlock();
	}
};

class C_SKLIST {
	NODE_SK head, tail;
	mutex ll;
public:
	C_SKLIST()
	{
		head.v = 0x80000000;
		tail.v = 0x7FFFFFFF;
		tail.top_level = head.top_level = MAX_LEVEL;
		for (auto& n : head.next) n = &tail;
	}
	void Find(int x, NODE_SK* pred[], NODE_SK* curr[])
	{
		pred[MAX_LEVEL] = &head;
		for (int i = MAX_LEVEL; i >= 0; --i) {
			curr[i] = pred[i]->next[i];
			while (curr[i]->v < x) {
				pred[i] = curr[i];
				curr[i] = curr[i]->next[i];
			}
			if (i == 0) break;
			pred[i - 1] = pred[i];
		}
	}
	bool ADD(int x)
	{
		// 검색
		NODE_SK* pred[MAX_LEVEL + 1];
		NODE_SK* curr[MAX_LEVEL + 1];
		ll.lock();
		Find(x, pred, curr);

		// 있나 없나 확인
		if (curr[0]->v == x) {
			ll.unlock();
			return false;
		}
		else { 	// 없으면 추가
			int new_level = 0;
			for (int i = 0; i < MAX_LEVEL; ++i) {
				if ((rand() % 2) == 0) break;
				new_level++;
			}
			NODE_SK* node = new NODE_SK{ x, new_level };
			for (int i = 0; i <= new_level; ++i)
				node->next[i] = curr[i];
			for (int i = 0; i <= new_level; ++i)
				pred[i]->next[i] = node;
			ll.unlock();
			return true;
		}
	}

	bool REMOVE(int x)
	{
		// 검색
		NODE_SK* pred[MAX_LEVEL + 1];
		NODE_SK* curr[MAX_LEVEL + 1];
		ll.lock();
		Find(x, pred, curr);

		// 있나 없나 확인
		if (curr[0]->v != x) {
			ll.unlock();
			return false;
		}
		else { 	// 있으면 삭제
			int max_level = curr[0]->top_level;
			for (int i = 0; i <= max_level; ++i)
				pred[i]->next[i] = curr[0]->next[i];
			delete curr[0];
			ll.unlock();
			return true;
		}
	}

	bool CONTAINS(int x)
	{
		// 검색
		NODE_SK* pred[MAX_LEVEL + 1];
		NODE_SK* curr[MAX_LEVEL + 1];
		ll.lock();
		Find(x, pred, curr);

		// 있나 없나 확인
		if (curr[0]->v == x) {
			ll.unlock();
			return true;
		}
		else {
			ll.unlock();
			return false;
		}
	}

	void print20()
	{
		NODE_SK* p = head.next[0];
		for (int i = 0; i < 20; ++i) {
			if (p == &tail) break;
			cout << p->v << ", ";
			p = p->next[0];
		}
		cout << endl;
	}

	void clear()
	{
		NODE_SK* p = head.next[0];
		while (p != &tail) {
			NODE_SK* t = p;
			p = p->next[0];
			delete t;
		}
		for (auto& n : head.next) n = &tail;
	}
};

class L_SKLIST {
	NODE_SK head, tail;
public:
	L_SKLIST()
	{
		head.v = 0x80000000;
		tail.v = 0x7FFFFFFF;
		tail.top_level = head.top_level = MAX_LEVEL;
		for (auto& n : head.next) n = &tail;
	}
	int Find(int x, NODE_SK* pred[], NODE_SK* curr[])
	{
		int lfound = -1;
		pred[MAX_LEVEL] = &head;
		for (int i = MAX_LEVEL; i >= 0; --i) {
			curr[i] = pred[i]->next[i];
			while (curr[i]->v < x) {
				pred[i] = curr[i];
				curr[i] = curr[i]->next[i];
			}
			if ((curr[i]->v == x) && (lfound == -1))
				lfound = i;
			if (i == 0) break;
			pred[i - 1] = pred[i];
		}
		return lfound;
	}
	bool ADD(int x)
	{
		// 검색
		NODE_SK* pred[MAX_LEVEL + 1];
		NODE_SK* curr[MAX_LEVEL + 1];

		while (true) {
			int lfound = Find(x, pred, curr);

			if (-1 != lfound) {
				if (true == curr[lfound]->removed)
					continue;
				while (curr[lfound]->fullylinked == false);
				return false;
			}

			// x값이 존재하지 않는다.
			// Lock을 하고 Valid검사를 하자.
			// 몽땅 Lock을 하고 Valid검사를 하면 필요없는 Lock을 추가로 할 수 있으니. 하나씩 Lock을 하면서 검사하자.
			int new_level = 0;
			for (int i = 0; i < MAX_LEVEL; ++i) {
				if ((rand() % 2) == 0) break;
				new_level++;
			}

			bool valid = true;
			int locked_top_level = 0;
			for (int i = 0; i <= new_level; ++i) {
				pred[i]->lock();
				locked_top_level = i;
				valid = (pred[i]->removed == false) && (curr[i]->removed == false) && (pred[i]->next[i] == curr[i]);
				if (false == valid) break;
			}
			if (false == valid) {
				for (int i = 0; i <= locked_top_level; ++i)
					pred[i]->unlock();
				continue;
			}

			NODE_SK* new_node = new NODE_SK{ x, new_level };
			for (int i = 0; i <= new_level; ++i)
				new_node->next[i] = curr[i];
			for (int i = 0; i <= new_level; ++i)
				pred[i]->next[i] = new_node;
			new_node->fullylinked = true;

			for (int i = 0; i <= locked_top_level; ++i)
				pred[i]->unlock();

			return true;
		}
	}

	bool REMOVE(int x)
	{
		NODE_SK* pred[MAX_LEVEL + 1];
		NODE_SK* curr[MAX_LEVEL + 1];

		bool removed = false;
		int new_level = -1;
		NODE_SK* node = nullptr;

		while (true)
		{
			int found = Find(x, pred, curr);

			if (found != -1)
				node = curr[found];

			if (removed == true
				or (found != -1
					and node->fullylinked == true
					and node->top_level == found
					and node->removed == false))
			{
				if (removed == false)
				{
					new_level = node->top_level;

					node->lock();

					if (node->removed == true)
					{
						node->unlock();
						return false;
					}

					node->removed = true;
					removed = true;
				}

				bool valid = true;
				int locked_top_level = -1;

				for (int i = 0; i <= new_level; ++i)
				{
					pred[i]->lock();

					locked_top_level = i;
					valid = pred[i]->removed == false and pred[i]->next[i] == node;

					if (valid == false)
						break;
				}

				if (valid == false)
				{
					/*if (removed == true)
					{
						node->removed = false;
						node->unlock();
					}*/

					for (int i = 0; i <= locked_top_level; ++i)
					{
						pred[i]->unlock();
					}

					continue;
				}

				for (int i = new_level; i >= 0; --i)
				{
					pred[i]->next[i] = node->next[i];
				}

				if (node != nullptr)
					node->unlock();

				for (int i = 0; i <= locked_top_level; ++i)
				{
					pred[i]->unlock();
				}

				return true;
			}
			else
				return false;
		}
	}

	bool CONTAINS(int x)
	{
		// 검색
		NODE_SK* pred[MAX_LEVEL + 1];
		NODE_SK* curr[MAX_LEVEL + 1];

		int found = Find(x, pred, curr);

		return found != -1 and curr[found]->fullylinked == true and curr[found]->removed == false;
	}

	void print20()
	{
		NODE_SK* p = head.next[0];
		for (int i = 0; i < 20; ++i) {
			if (p == &tail) break;
			cout << p->v << ", ";
			p = p->next[0];
		}
		cout << endl;
	}

	void clear()
	{
		NODE_SK* p = head.next[0];
		while (p != &tail) {
			NODE_SK* t = p;
			p = p->next[0];
			delete t;
		}
		for (auto& n : head.next) n = &tail;
	}
};

class LF_NODE_SK;

class LF_PTR_SK {
	uint64_t next;
public:
	LF_PTR_SK() : next(0) {}
	LF_PTR_SK(bool marking, LF_NODE_SK* ptr)
	{
		next = reinterpret_cast<uint64_t>(ptr);

		if (true == marking)
			next = next | 1;
	}
	LF_NODE_SK* get_ptr()
	{
		return reinterpret_cast<LF_NODE_SK*>(next & 0xFFFFFFFFFFFFFFFE);
	}
	bool get_removed()
	{
		return (next & 1) == 1;
	}
	LF_NODE_SK* get_ptr_mark(bool* removed)
	{
		unsigned long long cur_next = next;
		*removed = (cur_next & 1) == 1;

		return reinterpret_cast<LF_NODE_SK*>(cur_next & 0xFFFFFFFFFFFFFFFE);
	}
	bool CAS(LF_NODE_SK* o_ptr, LF_NODE_SK* n_ptr, bool o_mark, bool n_mark)
	{
		uint64_t o_next = reinterpret_cast<uint64_t>(o_ptr);
		uint64_t n_next = reinterpret_cast<uint64_t>(n_ptr);

		if (true == o_mark)
			++o_next;

		if (true == n_mark)
			++n_next;

		return atomic_compare_exchange_strong(
			reinterpret_cast<atomic_uint64_t*>(&next), &o_next, n_next);
	}
	bool is_removed()
	{
		return (next & 1) == 1;
	}
};

class LF_NODE_SK {
public:
	int v;
	LF_PTR_SK next[MAX_LEVEL + 1];
	int	top_level;
	LF_NODE_SK() : v(-1), top_level(0) { }
	LF_NODE_SK(int x, int top) : v(x), top_level(top) {}
};

class LF_SKLIST {
	LF_NODE_SK head, tail;
public:
	LF_SKLIST()
	{
		head.v = 0x80000000;
		tail.v = 0x7FFFFFFF;

		tail.top_level = head.top_level = MAX_LEVEL;

		for (auto& n : head.next)
		{
			n = LF_PTR_SK{ false, &tail };
		}
	}

	bool Find(int x, LF_NODE_SK* pred[], LF_NODE_SK* curr[])
	{
	restart:

		pred[MAX_LEVEL] = &head;

		for (int i = MAX_LEVEL; i >= 0; --i)
		{
			curr[i] = pred[i]->next[i].get_ptr();

			while (true)
			{
				bool removed;
				LF_NODE_SK* succ = curr[i]->next[i].get_ptr_mark(&removed);

				while (true == removed)
				{
					if (false == pred[i]->next[i].CAS(curr[i], succ, false, false))
						goto restart;

					curr[i] = succ;
					succ = curr[i]->next[i].get_ptr_mark(&removed);
				}

				if (curr[i]->v < x)
				{
					pred[i] = curr[i];
					curr[i] = succ;
				}
				else
				{
					if (i == 0)
						return curr[0]->v == x;

					pred[i - 1] = pred[i];
					break;
				}
			}
		}
	}

	bool ADD(int x)
	{
		// 검색
		LF_NODE_SK* pred[MAX_LEVEL + 1];
		LF_NODE_SK* curr[MAX_LEVEL + 1];

		while (true) {
			bool exist = Find(x, pred, curr);

			if (true == exist)
				return false;

			// x값이 존재하지 않는다.

			int new_level = 0;

			for (int i = 0; i < MAX_LEVEL; ++i)
			{
				if ((rand() % 2) == 0)
					break;

				new_level++;
			}

			LF_NODE_SK* new_node = new LF_NODE_SK{ x, new_level };

			for (int i = 0; i <= new_level; ++i)
			{
				new_node->next[i] = LF_PTR_SK{ false, curr[i] };
			}

			if (false == pred[0]->next[0].CAS(curr[0], new_node, false, false))
				continue;

			for (int i = 1; i <= new_level; ++i)
			{
				while (false == pred[i]->next[i].CAS(curr[i], new_node, false, false))
				{
					Find(x, pred, curr);
				}
			}

			return true;
		}
	}

	bool REMOVE(int x)
	{
		LF_NODE_SK* pred[MAX_LEVEL + 1];
		LF_NODE_SK* curr[MAX_LEVEL + 1];

		while (true)
		{
			if (Find(x, pred, curr) == false)
				return false;

			// x값이 존재하지 않는다.
			LF_NODE_SK* node = curr[0];
			int top_level = node->top_level;

			for (int i = top_level; i >= 1; --i)
			{
				bool removed = false;
				LF_NODE_SK* success = node->next[i].get_ptr_mark(&removed);

				while (removed == false)
				{
					node->next[i].CAS(success, success, false, true);
					success = node->next[i].get_ptr_mark(&removed);
				}
			}

			bool removed = false;

			while (true)
			{
				LF_NODE_SK* success = node->next[0].get_ptr_mark(&removed);

				if (node->next[0].CAS(success, success, false, true) == false)
				{
					if (removed == true)
						return false;

					continue;
				}

				Find(x, pred, curr);
				return true;
			}
		}
	}

	bool CONTAINS(int x)
	{
		LF_NODE_SK* pred[MAX_LEVEL + 1];
		LF_NODE_SK* curr[MAX_LEVEL + 1];

		return Find(x, pred, curr);
	}

	void print20()
	{
		LF_NODE_SK* p = head.next[0].get_ptr();

		for (int i = 0; i < 20; ++i)
		{
			if (p == &tail)
				break;

			cout << p->v << ", ";
			p = p->next[0].get_ptr();
		}

		cout << endl;
	}

	void clear()
	{
		LF_NODE_SK* p = head.next[0].get_ptr();

		while (p != &tail)
		{
			LF_NODE_SK* t = p;
			p = p->next[0].get_ptr();

			delete t;
		}

		for (auto& n : head.next)
		{
			n = LF_PTR_SK{ false, &tail };
		}
	}
};

//SET my_set;   // 성긴 동기화
// F_SET my_set;   // 세밀한 동기화
//O_SET my_set;	// 낙천적 동기화
//L_SET my_set;	// 게으른 동기화
//L_SET_SP my_set; // 게으른 동기화 shared_ptr 구현
//LF_SET my_set;
//C_SKLIST my_set;
//L_SKLIST my_set;
LF_SKLIST my_set;

class HISTORY {
public:
	int op;
	int i_value;
	bool o_value;
	HISTORY(int o, int i, bool re) : op(o), i_value(i), o_value(re) {}
};

constexpr int RANGE = 1000;

void worker(vector<HISTORY>* history, int num_threads)
{
	for (int i = 0; i < 4000000 / num_threads; ++i) {
		int op = rand() % 3;
		switch (op) {
			case 0: {
				int v = rand() % RANGE;
				my_set.ADD(v);
				break;
			}
			case 1: {
				int v = rand() % RANGE;
				my_set.REMOVE(v);
				break;
			}
			case 2: {
				int v = rand() % RANGE;
				my_set.CONTAINS(v);
				break;
			}
		}
	}
}

void worker_check(vector<HISTORY>* history, int num_threads)
{
	for (int i = 0; i < 4000000 / num_threads; ++i) {
		int op = rand() % 3;
		switch (op) {
			case 0: {
				int v = rand() % RANGE;
				history->emplace_back(0, v, my_set.ADD(v));
				break;
			}
			case 1: {
				int v = rand() % RANGE;
				history->emplace_back(1, v, my_set.REMOVE(v));
				break;
			}
			case 2: {
				int v = rand() % RANGE;
				history->emplace_back(2, v, my_set.CONTAINS(v));
				break;
			}
		}
	}
}

void check_history(array <vector <HISTORY>, 16>& history, int num_threads)
{
	array <int, RANGE> survive = {};
	cout << "Checking Consistency : ";
	if (history[0].size() == 0) {
		cout << "No history.\n";
		return;
	}
	for (int i = 0; i < num_threads; ++i) {
		for (auto& op : history[i]) {
			if (false == op.o_value) continue;
			if (op.op == 3) continue;
			if (op.op == 0) survive[op.i_value]++;
			if (op.op == 1) survive[op.i_value]--;
		}
	}
	for (int i = 0; i < RANGE; ++i) {
		int val = survive[i];
		if (val < 0) {
			cout << "The value " << i << " removed while it is not in the set.\n";
			exit(-1);
		}
		else if (val > 1) {
			cout << "The value " << i << " is added while the set already have it.\n";
			exit(-1);
		}
		else if (val == 0) {
			if (my_set.CONTAINS(i)) {
				cout << "The value " << i << " should not exists.\n";
				exit(-1);
			}
		}
		else if (val == 1) {
			if (false == my_set.CONTAINS(i)) {
				cout << "The value " << i << " shoud exists.\n";
				exit(-1);
			}
		}
	}
	cout << " OK\n";
}

int main()
{
	for (int num_threads = 1; num_threads <= 16; num_threads *= 2) {
		vector <thread> threads;
		array<vector <HISTORY>, 16> history;
		my_set.clear();
		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(worker_check, &history[i], num_threads);
		for (auto& th : threads)
			th.join();
		auto end_t = high_resolution_clock::now();
		auto exec_t = end_t - start_t;
		auto exec_ms = duration_cast<milliseconds>(exec_t).count();
		my_set.print20();
		cout << num_threads << " Threads.  Exec Time : " << exec_ms << endl;
		check_history(history, num_threads);
	}

	cout << "======== SPEED CHECK =============\n";

	for (int num_threads = 1; num_threads <= 16; num_threads *= 2) {
		vector <thread> threads;
		array<vector <HISTORY>, 16> history;
		my_set.clear();
		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(worker, &history[i], num_threads);
		for (auto& th : threads)
			th.join();
		auto end_t = high_resolution_clock::now();
		auto exec_t = end_t - start_t;
		auto exec_ms = duration_cast<milliseconds>(exec_t).count();
		my_set.print20();
		cout << num_threads << " Threads.  Exec Time : " << exec_ms << endl;
		check_history(history, num_threads);
	}
}