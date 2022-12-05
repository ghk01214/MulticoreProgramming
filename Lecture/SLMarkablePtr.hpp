#pragma once

template<typename T>
class SLNode;

template<typename T>
class SLMarkablePtr
{
public:
	SLMarkablePtr();
	SLMarkablePtr(bool mark, SLNode<T>* next);

	SLNode<T>* get_ptr() { return reinterpret_cast<SLNode<T>*>(address & 0xFFFFFFFFFFFFFFFE); }
	bool get_removed() { return (address & 1) == 1; }
	SLNode<T>* get_ptr_n_mark(bool* removed);
	bool try_change_mark(SLNode<T>* node, bool removed) { return cas(node, node, false, true); }

	bool cas(SLNode<T>* old_ptr, bool old_mark, SLNode<T>* new_ptr,bool new_mark);

private:
	uint64_t address;
};

template<typename T>
inline SLMarkablePtr<T>::SLMarkablePtr() :
	address{ 0 }
{
}

template<typename T>
inline SLMarkablePtr<T>::SLMarkablePtr(bool mark, SLNode<T>* addr)
{
	address = reinterpret_cast<uint64_t>(addr);

	if (mark == true)
		address = address | 1;
}

template<typename T>
inline SLNode<T>* SLMarkablePtr<T>::get_ptr_n_mark(bool* removed)
{
	uint64_t current_next{ address };

	*removed = (current_next & 1) == 1;

	return reinterpret_cast<SLNode<T>*>(current_next & 0xFFFFFFFFFFFFFFFE);
}

template<typename T>
inline bool SLMarkablePtr<T>::cas(SLNode<T>* old_ptr, bool old_mark, SLNode<T>* new_ptr, bool new_mark)
{
	uint64_t old_next{ reinterpret_cast<uint64_t>(old_ptr) };
	if (old_mark == true)
		++old_next;

	uint64_t new_next{ reinterpret_cast<uint64_t>(new_ptr) };
	if (new_mark == true)
		++new_next;

	return std::atomic_compare_exchange_strong(reinterpret_cast<std::atomic_uint64_t*>(&address), &old_next, new_next);
}
