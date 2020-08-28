#pragma once
#include <algorithm>
#include <memory>
#include <vector>
#include <variant>
#include <range/v3/all.hpp>


template<typename T, int sbo_size>
struct sbo_vector {

	sbo_vector() {
		//can't =default;?
	};

	template<typename I,typename S>
	sbo_vector(I&& i,S&& s) {
		reserve(ranges::distance(i, s));
		std::uninitialized_copy(i, s, begin());
	}

	sbo_vector(const sbo_vector& other) {
		reserve(other.size());
		std::uninitialized_copy(other.begin(), other.end(), begin());
		m_size = other.m_size;
	}

	sbo_vector(sbo_vector&& other) noexcept {
		if (other.is_sbo()) {
			std::uninitialized_move(other.begin(), other.end(), begin());
			m_size = other.size();
		}
		else {
			m_data = std::exchange(other.m_data, nullptr);
			m_size = std::exchange(other.m_size);
			other.m_capacity = sbo_size;
		}
	}

	sbo_vector& operator=(const sbo_vector& other) {
		if (this == &other) {
			return *this;
		}
		reserve(other.size());
		std::uninitialized_copy(other.begin(), other.end(), begin());
		m_size = other.m_size;
		return *this;
	}

	sbo_vector& operator=(sbo_vector&& other) noexcept {
		if (this == &other) {
			return *this;
		}
		if (other.is_sbo()) {
			std::uninitialized_move(other.begin(), other.end(), begin());
			m_size = other.size();
		}
		else {
			m_data = std::exchange(other.m_data, nullptr);
			m_size = std::exchange(other.m_size);
			other.m_capacity = sbo_size;
		}
		return *this;
	}

	~sbo_vector() {
		clear();
	}

	void reserve(int n) {
		if (n <= m_capacity) {//n<=m_capacity -> n<=sbo_size
			return;
		}
		else {//n > sbo_size here
			T* new_data = (T*)malloc(n * sizeof(T));
			std::uninitialized_move(begin(), end(), new_data);
			std::destroy(begin(), end());
			free(m_data);
			m_data = new_data;
		}
	}

	void clear() {
		m_size = 0;
		std::destroy(begin(), end());
	}

	bool is_sbo() const noexcept {
		return m_capacity == sbo_size;
	}

	T* begin() {
		if (is_sbo()) {
			return std::launder((T*)m_storage);
		}
		else {
			return m_data;
		}
	}

	T* end() {
		return begin() + m_size;
	}

	const T* begin() const {
		if (is_sbo()) {
			return std::launder((T*)m_storage);
		}
		else {
			return m_data;
		}
	}

	const T* end() const {
		return begin() + m_size;
	}

	size_t size() const noexcept {
		return (size_t)m_size;
	}

	bool empty() const noexcept {
		return size() == 0;
	}

	T* data() {
		return begin();
	}

	const T* data() const{
		return begin();
	}


	void push_back(T thing) {
		if (size() == capacity()) {
			reserve(std::max((size_t)1, capacity() * 2));
		}
		new(begin()[m_size++])T(std::move(thing));
	}

	template<typename... Args>
	T& emplace_back(Args&& ...args) {
		if (size() == capacity()) {
			reserve(std::max((size_t)1, capacity() * 2));
		}
		return *new(begin()[m_size++])T(std::forward<Args>(args)...);
	}

	void pop_back() {
		--m_size;
		std::destroy_at(m_data + m_size);
	}

	size_t capacity() const noexcept {
		return m_capacity;
	}

	void resize(size_t new_size) {
		if(new_size<=m_size) {
			std::destroy_at(begin() + new_size, begin() + size());
			m_size = new_size;
		}else {
			reserve(new_size);
			std::uninitialized_default_construct(begin() + size(), begin() + new_size);
			m_size = new_size;
		}		
	}

	void resize(size_t new_size,T val) {
		if (new_size <= m_size) {
			std::destroy_at(begin() + new_size, begin() + size());
		}
		else {
			reserve(new_size);
			std::uninitialized_fill(begin() + size(), begin() + new_size, val);
			m_size = new_size;
		}
	}

private:

	union {
		alignas(T) std::array<std::byte, sbo_size * sizeof(T)> m_storage = {};

		T* m_data;
	};
	//these are ints to save space? ;-;
	int m_capacity = sbo_size;
	int m_size = 0;

};
