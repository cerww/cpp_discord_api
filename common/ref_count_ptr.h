#pragma once
#include <atomic>
#include <compare>
#include <concepts>

struct ref_counted {
	ref_counted() = default;

	ref_counted(const ref_counted& o) {}

	ref_counted(ref_counted&& o) noexcept {}

	ref_counted& operator=(const ref_counted& o) {
		return *this;
	}

	ref_counted& operator=(ref_counted&& o) noexcept {
		return *this;
	}

	~ref_counted() = default;
private:
	void increment_ref_count() const noexcept {
		m_ref_count.fetch_add(1, std::memory_order_relaxed);
	}
	
	//returns weather or not this shuold be deleted
	bool decrement_ref_count() const noexcept {
		return m_ref_count.fetch_sub(1, std::memory_order_relaxed) == 1;
	}

	int ref_count() const noexcept {
		return m_ref_count;
	}

	mutable std::atomic<int> m_ref_count = 0;//so i can have ref_count_ptr<const T>

	template<typename>
	friend struct ref_count_ptr;
};

struct ref_counted_thread_unsafe {
	ref_counted_thread_unsafe() = default;

	ref_counted_thread_unsafe(const ref_counted_thread_unsafe& o) {}

	ref_counted_thread_unsafe(ref_counted&& o) noexcept {}

	ref_counted_thread_unsafe& operator=(const ref_counted_thread_unsafe& o) {
		return *this;
	}

	ref_counted_thread_unsafe& operator=(ref_counted_thread_unsafe&& o) noexcept {
		return *this;
	}

	~ref_counted_thread_unsafe() = default;
private:
	void increment_ref_count() const noexcept {
		++m_ref_count;
	}
	
	//returns weather or not this shuold be deleted
	bool decrement_ref_count() const noexcept {
		return --m_ref_count == 0;
	}

	int ref_count() const noexcept {
		return m_ref_count;
	}

	mutable int m_ref_count = 0;//so i can have ref_count_ptr<const T>

	template<typename>
	friend struct ref_count_ptr;
};

/// @brief intrusive ptr
/// @tparam T 
template<typename T>
struct ref_count_ptr {
	static_assert(!std::is_reference_v<T>);
	ref_count_ptr() = default;

	ref_count_ptr(nullptr_t) noexcept {}

	ref_count_ptr(T* t) noexcept :
		m_self(t) {
		
		if (m_self) {
			m_self->increment_ref_count();
		}
	}

	ref_count_ptr(const ref_count_ptr& other) noexcept :
		m_self(other.m_self) {
		
		if (m_self) {
			m_self->increment_ref_count();
		}
	}

	ref_count_ptr(ref_count_ptr&& other) noexcept :
		m_self(std::exchange(other.m_self, nullptr)) {}

	template<typename O, std::enable_if_t<std::is_base_of_v<O, T>, int>  = 0>
	explicit ref_count_ptr(const ref_count_ptr<O>& o) :
		m_self(o.m_self) {
		if (m_self) {
			m_self->increment_ref_count();
		}
	}

	template<typename O, std::enable_if_t<std::is_base_of_v<T, O>, int>  = 0>
	ref_count_ptr(const ref_count_ptr<O>& o) :
		m_self(o.m_self) {
		if (m_self) {
			m_self->increment_ref_count();
		}
	}

	~ref_count_ptr() noexcept {
		if (m_self && m_self->decrement_ref_count()) {
			delete m_self;
		}
	}

	ref_count_ptr& operator=(T* other) noexcept {
		ref_count_ptr n(other);
		std::swap(n.m_self, m_self);
		return *this;
	}

	ref_count_ptr& operator=(const ref_count_ptr& other) noexcept {
		ref_count_ptr n(other);
		std::swap(n.m_self, m_self);
		return *this;
	}

	ref_count_ptr& operator=(ref_count_ptr&& other) noexcept {
		ref_count_ptr temp(std::move(other));
		std::swap(m_self, temp.m_self);
		return *this;
	}

	operator ref_count_ptr<const T>() const noexcept {
		return ref_count_ptr<const T>(m_self);
	}

	T* get() const noexcept {
		return m_self;
	}

	T* operator->() const noexcept {
		return m_self;
	}

	T& operator*() const noexcept {
		return *m_self;
	}

	operator bool() const noexcept {
		return m_self;
	}

	bool operator==(nullptr_t) const noexcept {
		return m_self == nullptr;
	}

	bool operator!=(nullptr_t) const noexcept {
		return m_self != nullptr;
	}

	constexpr bool operator==(const ref_count_ptr&) const noexcept = default;

	//keep this?
	constexpr std::strong_ordering operator<=>(const ref_count_ptr&) const noexcept = default;
	
private:
	T* m_self = nullptr;
};

template<typename T, typename... args>requires std::constructible_from<T,args...>
ref_count_ptr<T> make_ref_count_ptr(args&&... Args) {
	return ref_count_ptr<T>(new T(std::forward<args>(Args)...));
}
