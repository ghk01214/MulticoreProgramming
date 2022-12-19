#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include <atomic>
#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <array>
//#include <sstream>

//#define LIST clist
//#define LIST flist
//#define LIST olist
//#define LIST zlist
//#define LIST lflist
#define LIST zXlist

using namespace std;
using namespace chrono;

#define NUM_TEST   4000000
#define KEY_RANGE  1000
mutex g_lock;
int num_thread;

static const int MAX_RECORD = 100;

array <atomic_int, MAX_RECORD> xrm_record;
atomic <int> max_index;
//concurrent_unordered_map <unsigned int, int> xrm_record;

unsigned int my_xbegin()
{
	unsigned int result = _xbegin();
	if (result == _XBEGIN_STARTED) return result;

	int index = 0;
	while (index < max_index) {
		if (xrm_record[index] == result) {
			xrm_record[index + 1]++;
			return result;
		}
		index += 2;
	}

	int new_index;
	do {
		new_index = index + 2;
		if (new_index > MAX_RECORD) { cout << "Too Many sort of Errors!!!\n"; exit(-1); }
	} while (false == atomic_compare_exchange_strong(&max_index, &index, new_index));
	xrm_record[index] = result;
	xrm_record[index + 1]++;
	return result;
}

void report_xrm()
{
	for (auto i=0;i<max_index;i+=2)
	{
		int error_type = xrm_record[i];
		cout << "Abort Count: " << xrm_record[i + 1] << " CODE : " << hex << error_type << dec << "   Abort Type :";
		if (error_type & _XABORT_EXPLICIT) printf("Explicit ");
		if (error_type & _XABORT_RETRY) printf("Retry ");
		if (error_type & _XABORT_CONFLICT) printf("Conflict ");
		if (error_type & _XABORT_CAPACITY) printf("Capacity ");
		if (error_type & _XABORT_DEBUG) printf("Debug  ");
		if (error_type & _XABORT_NESTED) printf("Nested ");
		cout << endl;
	};
}

class NODE {
public:
	int key;
	int item;
	bool marked;
	mutex node_lock;
	NODE *next;

	NODE() {
		next = NULL;
		marked = false;
	}

	NODE(int key_value) {
		next = NULL;
		key = key_value;
		marked = false;
	}

	~NODE() {
	}

	void lock() {
		node_lock.lock();
	}

	void unlock() {
		node_lock.unlock();
	}

};

class CLIST {
	NODE head, tail;
	mutex glock;
public:
	CLIST()
	{
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.next = &tail;
	}
	~CLIST() {}

	void Init()
	{
		NODE *ptr;
		while (head.next != &tail) {
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}
	bool Add(int key)
	{
		NODE *pred, *curr;

		pred = &head;
		glock.lock();
		curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		if (key == curr->key) {
			glock.unlock();
			return false;
		}
		else {
			NODE *node = new NODE(key);
			node->next = curr;
			pred->next = node;
			glock.unlock();
			return true;
		}
	}
	bool Remove(int key)
	{
		NODE *pred, *curr;

		pred = &head;
		glock.lock();
		curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		if (key == curr->key) {
			pred->next = curr->next;
			delete curr;
			glock.unlock();
			return true;
		}
		else {
			glock.unlock();
			return false;
		}
	}
	bool Contains(int key)
	{
		NODE *pred, *curr;

		pred = &head;
		glock.lock();
		curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}
		if (key == curr->key) {
			glock.unlock();
			return true;
		}
		else {
			glock.unlock();
			return false;
		}
	}
};

CLIST clist;

class FLIST {
	NODE head, tail;
public:
	FLIST()
	{
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.next = &tail;
	}
	~FLIST() {}

	void Init()
	{
		NODE *ptr;
		while (head.next != &tail) {
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}

	bool Add(int key)
	{
		NODE *pred, *curr;

		head.lock();
		pred = &head;
		curr = pred->next;
		curr->lock();
		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (key == curr->key) {
			curr->unlock();
			pred->unlock();
			return false;
		}
		else {
			NODE *node = new NODE(key);
			node->next = curr;
			pred->next = node;
			curr->unlock();
			pred->unlock();
			return true;
		}
	}
	bool Remove(int key)
	{
		NODE *pred, *curr;

		head.lock();
		pred = &head;
		curr = pred->next;
		curr->lock();
		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (key == curr->key) {
			pred->next = curr->next;
			curr->unlock();
			pred->unlock();
			delete curr;
			return true;
		}
		else {
			curr->unlock();
			pred->unlock();
			return false;
		}
	}
	bool Contains(int key)
	{
		NODE *pred, *curr;

		head.lock();
		pred = &head;
		curr = pred->next;
		curr->lock();
		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}
		if (key == curr->key) {
			curr->unlock();
			pred->unlock();
			return true;
		}
		else {
			curr->unlock();
			pred->unlock();
			return false;
		}
	}
};

FLIST flist;

class OLIST {
	NODE head, tail;
public:
	OLIST()
	{
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.next = &tail;
	}
	~OLIST() {}

	void Init()
	{
		NODE *ptr;
		while (head.next != &tail) {
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}

	bool validate(NODE *pred, NODE *curr) {
		NODE *node = &head;
		while (node->key <= pred->key) {
			if (node == pred) return pred->next == curr;
			node = node->next;
		}
		return false;
	}

	bool Add(int key)
	{
		NODE *pred, *curr;

		while (true) {
			pred = &head;
			curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}
			pred->lock();
			curr->lock();

			if (!validate(pred, curr)) {
				curr->unlock();
				pred->unlock();
				continue;
			}

			if (key == curr->key) {
				curr->unlock();
				pred->unlock();
				return false;
			}
			else {
				NODE *node = new NODE(key);
				node->next = curr;
				pred->next = node;
				curr->unlock();
				pred->unlock();
				return true;
			}
		}
	}

	bool Remove(int key)
	{
		NODE *pred, *curr;

		while (true) {
			pred = &head;
			curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}
			pred->lock();
			curr->lock();

			if (!validate(pred, curr)) {
				curr->unlock();
				pred->unlock();
				continue;
			}
			if (key == curr->key) {
				pred->next = curr->next;
				curr->unlock();
				pred->unlock();
				// delete curr;
				return true;
			}
			else {
				curr->unlock();
				pred->unlock();
				return false;
			}
		}
	}

	bool Contains(int key)
	{
		NODE *pred, *curr;

		while (true) {
			pred = &head;
			curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}
			pred->lock();
			curr->lock();
			if (!validate(pred, curr)) {
				curr->unlock();
				pred->unlock();
				continue;
			}
			if (key == curr->key) {
				curr->unlock();
				pred->unlock();
				return true;
			}
			else {
				curr->unlock();
				pred->unlock();
				return false;
			}
		}
	}
};

OLIST olist;

class ZLIST {
	NODE head, tail;
public:
	ZLIST()
	{
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.next = &tail;
	}
	~ZLIST() {}

	void Init()
	{
		NODE *ptr;
		while (head.next != &tail) {
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}

	void PrintFirst20()
	{
		NODE* ptr = head.next;

		cout << "Printing First 20 values : ";
		for (int i = 0; i < 20; ++i) {
			if (ptr == &tail) break;
			cout << ptr->key << " ";
			ptr = ptr->next;
		}
		cout << endl;
	}

	bool validate(NODE *pred, NODE *curr) {
		return (!pred->marked) && (!curr->marked) && (pred->next == curr);
	}

	bool Add(int key)
	{
		NODE *pred, *curr;

		while (true) {
			pred = &head;
			curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}
			pred->lock();
			curr->lock();

			if (!validate(pred, curr)) {
				curr->unlock();
				pred->unlock();
				continue;
			}

			if (key == curr->key) {
				curr->unlock();
				pred->unlock();
				return false;
			}
			else {
				NODE *node = new NODE(key);
				node->next = curr;
				pred->next = node;
				curr->unlock();
				pred->unlock();
				return true;
			}
		}
	}

	bool Remove(int key)
	{
		NODE *pred, *curr;

		while (true) {
			pred = &head;
			curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}
			pred->lock();
			curr->lock();

			if (!validate(pred, curr)) {
				curr->unlock();
				pred->unlock();
				continue;
			}
			if (key == curr->key) {
				curr->marked = true;
				pred->next = curr->next;
				curr->unlock();
				pred->unlock();
				// delete curr;
				return true;
			}
			else {
				curr->unlock();
				pred->unlock();
				return false;
			}
		}
	}

	bool Contains(int key)
	{
		NODE *curr = &head;

		while (curr->key < key) curr = curr->next;
		return (curr->key == key) && !curr->marked;
	}
};

ZLIST zlist;

class LFNODE {
public:
	int key;
	int item;
	LFNODE *next;

	LFNODE() {
		next = NULL;
	}

	LFNODE(int key_value) {
		next = NULL;
		key = key_value;
	}

	~LFNODE() {
	}

	bool CompareAndSet(long long old_v, long long new_v)
	{
		return atomic_compare_exchange_strong(reinterpret_cast<atomic<long long> *>(&next), &old_v, new_v);
	}


	bool CAS(LFNODE *old_node, LFNODE *new_node, bool oldMark, bool newMark) {
		long long oldvalue = reinterpret_cast<long long>(old_node);
		if (oldMark) oldvalue = oldvalue | 0x01;
		else oldvalue = oldvalue & 0xFFFFFFFFFFFFFFFE;

		long long newvalue = reinterpret_cast<long long>(new_node);
		if (newMark) newvalue = newvalue | 0x01;
		else newvalue = newvalue & 0xFFFFFFFFFFFFFFFE;

		return CompareAndSet(oldvalue, newvalue);
	}

	bool AttemptMark(LFNODE *old_node, bool newMark) {
		long long oldvalue = reinterpret_cast<long long>(old_node);
		long long newvalue = oldvalue;
		if (newMark) newvalue = newvalue | 0x01;
		else newvalue = newvalue & 0xFFFFFFFFFFFFFFFE;
		return CompareAndSet(oldvalue, newvalue);
	}

	LFNODE *GetNextWithMark(bool *mark) {
		long long temp = reinterpret_cast<long long>(next);
		*mark = (0 != (temp & 0x01));
		return reinterpret_cast<LFNODE *>(temp & 0xFFFFFFFFFFFFFFFE);
	}

	LFNODE *GetReference()
	{
		long long temp = reinterpret_cast<long long>(next);
		return reinterpret_cast<LFNODE *>(temp & 0xFFFFFFFFFFFFFFFE);
	}

};

LFNODE *AtomicMarkableReference(LFNODE *ptr, bool mark)
{
	long long temp = reinterpret_cast<long long>(ptr);
	if (mark) temp = temp | 0x01;
	else temp = temp & 0xFFFFFFFFFFFFFFFE;
	return reinterpret_cast<LFNODE *>(temp);
}

class LFLIST {
	LFNODE head, tail;
public:
	LFLIST()
	{
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.next = &tail;
	}
	~LFLIST() {}

	void Init()
	{
		LFNODE *ptr;
		while (head.next != &tail) {
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}

	void PrintFirst20()
	{
		LFNODE *ptr = head.GetReference();

		cout << "Printing First 20 values : ";
		for (int i = 0; i < 20; ++i) {
			if (ptr == &tail) break;
			cout << ptr->key << " ";
			ptr = ptr->GetReference();
		}
		cout << endl;
	}

	void Find(LFNODE **Pred, LFNODE **Curr, int key)
	{
		LFNODE *pred = NULL;
		LFNODE *curr = NULL;
		LFNODE *succ = NULL;
	ng_retry:
		while (true) {
			pred = &head;
			curr = pred->GetReference();
			while (true) {
				bool marked;
				succ = curr->GetNextWithMark(&marked);
				while (marked) {
					if (false == pred->CAS(curr, succ, false, false)) goto ng_retry;
					curr = succ;
					succ = curr->GetNextWithMark(&marked);
				}
				if (curr->key >= key) {
					*Pred = pred;
					*Curr = curr;
					return;
				}
				pred = curr;
				curr = succ;
			}
		}
	}

	bool Add(int key)
	{
		LFNODE *pred, *curr;

		LFNODE *node = new LFNODE(key);
		while (true) {
			Find(&pred, &curr, key);

			if (key == curr->key) {
				delete node;
				return false;
			}
			node->next = AtomicMarkableReference(curr, false);
			if (pred->CAS(curr, node, false, false)) return true;
		}
	}

	bool Remove(int key)
	{
		LFNODE *pred, *curr;

		bool snip;
		while (true) {
			Find(&pred, &curr, key);

			if (key != curr->key) return false;
			LFNODE *succ = curr->GetReference();
			snip = curr->AttemptMark(succ, true);
			if (!snip) continue;
			pred->CAS(curr, succ, false, false);
			return true;
		}
	}

	bool Contains(int key)
	{
		bool marked = false;
		LFNODE *curr = &head;

		while (curr->key < key) curr = curr->GetNextWithMark(&marked);
		return (curr->key == key) && !marked;
	}
};

LFLIST lflist;

class XLIST {
	NODE head, tail;
	mutex glock;
public:
	XLIST()
	{
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.next = &tail;
	}
	~XLIST() {}

	void Init()
	{
		NODE *ptr;
		while (head.next != &tail) {
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}
	bool Add(int key)
	{
		NODE *pred, *curr;

		pred = &head;
		while (_XBEGIN_STARTED != my_xbegin()) _xabort(0);
		curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		if (key == curr->key) {
			_xend();
			return false;
		}
		else {
			NODE *node = new NODE(key);
			node->next = curr;
			pred->next = node;
			_xend();
			return true;
		}
	}
	bool Remove(int key)
	{
		NODE *pred, *curr;

		pred = &head;
		while (_XBEGIN_STARTED != my_xbegin()) _xabort(0);
		curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		if (key == curr->key) {
			pred->next = curr->next;
			delete curr;
			_xend();
			return true;
		}
		else {
			_xend();
			return false;
		}
	}
	bool Contains(int key)
	{
		NODE *pred, *curr;

		pred = &head;
		while (_XBEGIN_STARTED != my_xbegin()) _xabort(0);
		curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}
		if (key == curr->key) {
			_xend();
			return true;
		}
		else {
			_xend();
			return false;
		}
	}
};

XLIST xlist;

class ZXLIST {
	NODE head, tail;
public:
	ZXLIST()
	{
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.next = &tail;
	}
	~ZXLIST() {}

	void Init()
	{
		NODE *ptr;
		while (head.next != &tail) {
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}

	void PrintFirst20()
	{
		NODE *ptr = head.next;

		cout << "Printing First 20 values : ";
		for (int i = 0; i < 20; ++i) {
			if (ptr == &tail) break;
			cout << ptr->key << " ";
			ptr = ptr->next;
		}
		cout << endl;
	}

	bool validate(NODE *pred, NODE *curr) {
		return (!pred->marked) && (!curr->marked) && (pred->next == curr);
	}

	bool Add(int key)
	{
		NODE *pred, *curr;

		NODE *node = new NODE(key);
		while (true) {
			pred = &head;
			curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}
			if (_XBEGIN_STARTED != my_xbegin()) continue;
			if (!validate(pred, curr)) {
				_xabort(0);
				continue;
			}
			if (key == curr->key) {
				_xend();
				delete node;
				return false;
			}
			else {
				node->next = curr;
				pred->next = node;
				_xend();
				return true;
			}
		}
	}

	bool Remove(int key)
	{
		NODE *pred, *curr;

		while (true) {
			pred = &head;
			curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			if (_XBEGIN_STARTED != my_xbegin()) continue;

			if (!validate(pred, curr)) {
				_xabort(0);
				continue;
			}
			if (key == curr->key) {
				curr->marked = true;
				pred->next = curr->next;
				_xend();
				//delete curr;
				return true;
			}
			else {
				_xend();
				return false;
			}
		}
	}

	bool Contains(int key)
	{
		NODE *curr = &head;

		while (curr->key < key) curr = curr->next;
		return (curr->key == key) && !curr->marked;
	}
};

ZXLIST zXlist;

void ThreadFunc(int num_thread)
{
	int key;

	for (int i = 0; i < NUM_TEST / num_thread; i++) {
		switch (rand() % 3) {
		case 0: key = rand() % KEY_RANGE;
			LIST.Add(key);
			break;
		case 1: key = rand() % KEY_RANGE;
			LIST.Remove(key);
			break;
		case 2: key = rand() % KEY_RANGE;
			LIST.Contains(key);
			break;
		default: printf("Error\n");
			exit(-1);

		}
	}
	return;
}

int main()
{
	thread my_thread[64];

	for (num_thread = 1; num_thread <= 64; num_thread *= 2) {
		LIST.Init();
		for (auto i = 0; i < max_index; ++i) xrm_record[i] = 0;
		max_index = 0;
		auto start_time = high_resolution_clock::now();
		for (int i = 0; i < num_thread; ++i)
			my_thread[i] = thread{ ThreadFunc, num_thread };
		for (int i = 0; i < num_thread; ++i)
			my_thread[i].join();
		auto du = high_resolution_clock::now() - start_time;
		LIST.PrintFirst20();
		cout << "Number of Threads " << num_thread << ", Time " << duration_cast<milliseconds>(du).count() << endl;
		report_xrm();
	}

}
