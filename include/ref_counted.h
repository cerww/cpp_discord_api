#pragma once
#include <atomic>

struct ref_counted {
	void increment_ref_count() const noexcept {
		m_ref_count.fetch_add(1, std::memory_order_relaxed);
	}
	bool decrement_ref_count()const noexcept {
		return m_ref_count.fetch_sub(1, std::memory_order_relaxed) == 1;
	}
	size_t ref_count() const noexcept {
		return m_ref_count;
	}

private:
	mutable std::atomic<size_t> m_ref_count = 0;//so i can have ref_count_ptr<const T>
};

struct ref_counted_thread_unsafe {
	void increment_ref_count() const noexcept {
		++m_ref_count;
	}
	bool decrement_ref_count()const noexcept {
		return --m_ref_count == 0;
	}
	size_t ref_count() const noexcept {
		return m_ref_count;
	}

private:
	mutable size_t m_ref_count = 0;//so i can have ref_count_ptr<const T>
};

template<typename T>
class ref_count_ptr {
public:
	ref_count_ptr() = default;

	ref_count_ptr(T* t)noexcept :m_self(t) {
		m_self->increment_ref_count();
	}

	ref_count_ptr(const ref_count_ptr& other) noexcept :m_self(other.m_self) {
		m_self->increment_ref_count();
	}

	ref_count_ptr(ref_count_ptr&& other) noexcept :m_self(std::exchange(other.m_self, nullptr)) {}

	template<typename O, std::enable_if_t<std::is_base_of_v<O, T>, int> = 0>
	explicit ref_count_ptr(const ref_count_ptr<O>& o) : m_self(o.m_self) {
		m_self->increment_ref_count();
	}

	template<typename O, std::enable_if_t<std::is_base_of_v<T, O>, int> = 0>
	ref_count_ptr(const ref_count_ptr<O>& o) : m_self(o.m_self) {
		m_self->increment_ref_count();
	}

	~ref_count_ptr() {
		if (m_self && m_self->decrement_ref_count())
			delete m_self;
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
		if (&other == this)//self move is bad ;-;
			return *this;
		this->~ref_count_ptr();
		m_self = std::exchange(other.m_self, nullptr);
		return *this;
	}

	operator ref_count_ptr<const T>() {
		return ref_count_ptr<const T>(m_self);
	}

	T* get()const noexcept {
		return m_self;
	}
	T* operator->() const noexcept { return m_self; }
	T& operator*() const noexcept { return *m_self; }
private:
	T * m_self = nullptr;
};

template<typename T, typename... args>
ref_count_ptr<T> make_ref_count_ptr(args&&... Args) {
	return ref_count_ptr<T>(new T(std::forward<args>(Args)...));
}
