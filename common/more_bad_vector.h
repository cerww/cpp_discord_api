#pragma once
#include <memory>
#include <algorithm>
#include <vector>
#include <ranges>
#include <stdexcept>
#include <range/v3/all.hpp>
#include <span>

template<typename T>
struct more_bad_vector {
	using value_type = T;
	using iterator = T*;
	using const_iterator = const T*;
	using reverse_iterator = std::reverse_iterator<T*>;
	using const_reverse_iterator = std::reverse_iterator<const T*>;

	more_bad_vector() = default;

	// template<typename U>requires !std::is_same_v<U, more_bad_vector<T>>
	// explicit more_bad_vector(const U& other) {
	// 	const auto size = std::size(other);
	// 	resize_uninitialize(size);
	// 	std::uninitialized_copy(other.begin(), other.end(), begin());
	// }

	template<typename I, typename S>
	explicit more_bad_vector(I&& begin_t, S&& end_t) {
		resize_uninitialize(ranges::distance(begin_t, end_t));
		std::uninitialized_copy(begin_t, end_t, this->begin());
	}

	// template<typename U>
	// explicit worse_vector(U other) {
	// 	const auto size = std::size(other);
	// 	resize_uninitialize(size);
	// 	std::uninitialized_move(other.begin(), other.end(), begin());
	// }
	//

	explicit more_bad_vector(size_t init_size) {
		resize_uninitialize(init_size);
		std::uninitialized_default_construct(begin(), end());
	}

	template<typename U>
	explicit more_bad_vector(size_t init_size,const U& init) {
		resize_uninitialize(init_size);
		std::uninitialized_fill(begin(), end(), init);
	}

	more_bad_vector(const more_bad_vector& other) {
		if (other.size()) {
			resize_uninitialize(other.size());
			std::uninitialized_copy(other.begin(), other.end(), begin());
		}
	}

	more_bad_vector& operator=(const more_bad_vector& other) {
		if (&other == this) {
			return *this;
		}
		clear();
		if (other.size()) {
			resize_uninitialize(other.size());
			std::uninitialized_copy(other.begin(), other.end(), begin());
		} else {
			m_data = nullptr;
		}
		return *this;
	}

	more_bad_vector(more_bad_vector&& other) noexcept:
		m_data(std::exchange(other.m_data, nullptr)) { }

	more_bad_vector& operator=(more_bad_vector&& other) noexcept {
		if (&other == this) {
			return *this;
		}
		clear();
		m_data = std::exchange(other.m_data, nullptr);
		return *this;
	}

	~more_bad_vector() {
		clear();
	}

	T* begin() {
		if (m_data) {
			return std::launder((T*)(m_data + 16));
		} else {
			return nullptr;
		}
	}

	T* end() {
		if (m_data) {
			return std::launder((T*)((m_data + 16) + size()));
		} else {
			return nullptr;
		}
	}

	const T* begin() const {
		if (m_data) {
			return std::launder((T*)(m_data + 16));
		} else {
			return nullptr;
		}
	}

	const T* end() const {
		if (m_data) {
			return std::launder((T*)((m_data + 16) + size()));
		} else {
			return nullptr;
		}
	}

	const T* cbegin() const {
		if (m_data) {
			return std::launder((T*)(m_data + 16));
		} else {
			return nullptr;
		}
	}

	const T* cend() const {
		if (m_data) {
			return std::launder((T*)((m_data + 16) + size()));
		} else {
			return nullptr;
		}
	}

	auto rbegin() {
		return std::make_reverse_iterator(end());
	}

	auto rend() {
		return std::make_reverse_iterator(begin());
	}

	auto rbegin() const {
		return std::make_reverse_iterator(end());
	}

	auto rend() const {
		return std::make_reverse_iterator(begin());
	}

	size_t size() const noexcept {
		if (m_data) {
			return *(size_t*)m_data;
		} else {
			return 0;
		}
	}

	bool empty() const noexcept {
		return m_data == nullptr;
	}

	template<typename range>
	void assign(const range& from) {
		clear();
		const auto size = std::size(from);
		resize_uninitialize(size);
		std::uninitialized_copy(from.begin(), from.end(), begin());
	}

	template<typename I, typename S>
	void assign(I&& i, S&& s) {
		const auto size = ranges::distance(i, s);
		resize_uninitialize(size);
		std::uninitialized_copy(i, s, begin());
	}

	// template<typename range>
	// requires std::is_rvalue_reference_v<range>//&& sized_range
	// void assign(range&& from) {
	// 	clear();
	// 	const auto size = std::size(from);
	// 	resize_uninitialize(size);
	// 	std::uninitialized_move(from.begin(), from.end(), begin());
	// }

	void assign(const more_bad_vector& from) {
		if (&from == this) {
			return;
		} else {
			clear();
			const auto size = std::size(from);
			resize_uninitialize(size);
			std::uninitialized_copy(from.begin(), from.end(), begin());
		}
	}

	void assign(more_bad_vector&& from) {
		if (&from == this) {
			return;
		} else {
			clear();
			const auto size = std::size(from);
			resize_uninitialize(size);
			std::uninitialized_move(from.begin(), from.end(), begin());
		}
	}

	void clear() {
		std::destroy(begin(), end());
		free(m_data);  // NOLINT
		m_data = nullptr;
	}

	T& operator[](size_t idx) {
		return begin()[idx];
	}

	const T& operator[](size_t idx) const {
		return begin()[idx];
	}

	T& at(size_t idx) {
		if (idx >= size()) {
			throw std::runtime_error(";-;");
		}
		return begin()[idx];
	}

	const T& at(size_t idx) const {
		if (idx >= size()) {
			throw std::runtime_error(";-;");
		}
		return begin()[idx];
	}

	T& front() {
		return *begin();
	}

	T& back() {
		return *(end() - 1);
	}

	const T& front() const {
		return *begin();
	}

	const T& back() const {
		return *(end() - 1);
	}

	T* data() {
		return begin();
	}

	const T* data() const {
		return begin();
	}

	//no emplace_back, insert, reserve, erase

	void resize(size_t new_size) {
		const auto my_size = size();
		if (new_size > my_size) {
			more_bad_vector other;
			other.resize_uninitialize(new_size);
			std::uninitialized_move(begin(), end(), other.begin());
			std::uninitialized_default_construct(other.begin() + my_size, other.begin() + new_size);
			std::swap(other.m_data, m_data);
		}else if(new_size < my_size) {
			more_bad_vector other;
			other.resize_uninitialize(new_size);
			std::uninitialized_move(begin(), begin() + new_size, other.begin());
			std::swap(other.m_data, m_data);
		}
	}

	template<typename U>
	void resize(size_t new_size,const U& init) {
		const auto my_size = size();
		if (new_size > my_size) {
			more_bad_vector other;
			other.resize_uninitialize(new_size);
			std::uninitialized_move(begin(), end(), other.begin());
			std::uninitialized_fill(other.begin() + my_size, other.begin() + new_size, init);
			std::swap(other.m_data, m_data);
		}
		else if (new_size < my_size) {
			more_bad_vector other;
			other.resize_uninitialize(new_size);
			std::uninitialized_move(begin(), begin() + new_size, other.begin());
			std::swap(other.m_data, m_data);
		}
	}

private:


	void resize_uninitialize(size_t size) {
		assert(m_data == nullptr);
		if (size != 0) {
			m_data = (std::byte*)malloc(size * sizeof(T) + 16); // NOLINT
			*(size_t*)m_data = size;
		}
	}

	std::byte* m_data = nullptr;
};


/*
 * 
 *
 * 
 */
