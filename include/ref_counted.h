#pragma once
#include <atomic>
#include <mutex>


struct ref_counted{
	mutable std::atomic<size_t> ref_count = 0;	
	void increment_ref_count() const {
		++ref_count;
	}
	void decrement_ref_count()const {
		--ref_count;
	}
};

struct ref_counted_thread_unsafe {
	mutable size_t ref_count = 0;	
	void increment_ref_count() const {	
		++ref_count;
	}
	void decrement_ref_count()const {	
		--ref_count;
	}
};

template<typename T>
class ref_count_ptr{
public:
	ref_count_ptr() = default;
	ref_count_ptr(T* t):m_self(t) {
		m_self->increment_ref_count();
	}
	ref_count_ptr(const ref_count_ptr& other) :m_self(other.m_self) {
		m_self->increment_ref_count();
	}
	ref_count_ptr(ref_count_ptr&& other) noexcept:m_self(other.m_self) {
		other.m_self = nullptr;
	}
	~ref_count_ptr() {
		if (!m_self) return;
		m_self->decrement_ref_count();
		if(m_self->ref_count == 0){
			delete m_self;
		}
	}
	ref_count_ptr& operator=(T* other) {
		this->~ref_count_ptr();
		m_self = other;	
		m_self->increment_ref_count();
		return *this;
	}
	ref_count_ptr& operator=(const ref_count_ptr& other) {
		this->~ref_count_ptr();
		m_self = other.m_self;
		m_self->increment_ref_count();
		return *this;
	}
	ref_count_ptr& operator=(ref_count_ptr&& other) noexcept {
		this->~ref_count_ptr();
		m_self = std::exchange(other.m_self,nullptr);
		return *this;
	}
	T* get() noexcept{
		return m_self;
	}
	const T* get()const noexcept {
		return m_self;
	}
	T* operator->() { return m_self; }
	T* operator*() { return m_self; }
	const T* operator->() const{ return m_self; }
	const T* operator*() const{ return m_self; }
private:
	T * m_self = nullptr;
};

template<typename T,typename... args>
ref_count_ptr<T> make_ref_count_ptr(args&&... Args) {
	return ref_count_ptr<T>(new T(Args...));
}
