#pragma once
#include <algorithm>
#include <memory>
#include <vector>
#include <variant>
#include <range/v3/all.hpp>


template<typename T, int sbo_size>
struct sbo_vector {
	using iterator = T*;
	using value_type = T;
	using reference = T&;
	using const_reference = const T&;
	using const_iterator = const T*;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	sbo_vector() {
		//can't =default;?
	};

	explicit sbo_vector(size_t init_size) {
		resize(init_size);
	}
	
	explicit sbo_vector(size_t init_size,const T& a) {
		resize(init_size,a);
	}

	template<typename I, typename S>
	sbo_vector(I&& i, S&& s) {
		reserve(ranges::distance(i, s));
		const auto aaa = ranges::distance(i, s);
		
		auto it = std::uninitialized_copy(i, s, begin());
		m_size = (int)std::distance(begin(), it);
	}

	sbo_vector(const sbo_vector& other) {
		reserve(other.size());
		std::uninitialized_copy(other.begin(), other.end(), begin());
		m_size = other.m_size;
	}

	sbo_vector(sbo_vector&& other) noexcept {
		if (other.is_sbo()) {
			std::uninitialized_move(other.begin(), other.end(), begin());
			m_size = (int)other.size();
		} else {
			m_data = std::exchange(other.m_data, nullptr);
			m_size = std::exchange(other.m_size, 0);
			m_capacity = std::exchange(other.m_capacity, sbo_size);
			other.m_capacity = sbo_size;
		}
	}

	sbo_vector& operator=(const sbo_vector& other) {
		if (this == &other) {
			return *this;
		}
		//die();
		clear();//don't need to call die() to avoid reallcate?
		reserve(other.size());
		std::uninitialized_copy(other.begin(), other.end(), begin());
		m_size = other.m_size;
		return *this;
	}

	sbo_vector& operator=(sbo_vector&& other) noexcept {
		if (this == &other) {
			return *this;
		}
		die();
		if (other.is_sbo()) {
			m_capacity = sbo_size;
			std::uninitialized_move(other.begin(), other.end(), begin());
			m_size = (int)other.size();
			other.clear();
		} else {
			m_data = std::exchange(other.m_data, nullptr);
			m_capacity = std::exchange(other.m_capacity, sbo_size);			
			m_size = std::exchange(other.m_size, 0);
			assert(m_capacity > sbo_size);
		}
		return *this;
	}

	~sbo_vector() {
		die();
	}

	void reserve(size_t n) {
		if (n <= (size_t)m_capacity) {//n<=m_capacity -> n<=sbo_size since m_capacity>=sbo_size 
			return;
		} else {//n > sbo_size here
			T* new_data = (T*)malloc(n * sizeof(T));// NOLINT
			std::uninitialized_move(begin(), end(), new_data);
			std::destroy(begin(), end());
			if (!is_sbo()) {
				free(m_data);  // NOLINT
			}
			m_data = new_data;
			m_capacity = (int)n;	
		}
	}

	void shrink_to_fit() {
		if (m_size <= sbo_size) {
			auto data = m_data;
			m_capacity = sbo_size;
			std::uninitialized_move(data, data + m_size, begin());
			if (!is_sbo()) {
				free(data); //NOLINT
			}
		} else {
			T* new_data = (T*)malloc(m_size * sizeof(T));// NOLINT
			std::uninitialized_move(begin(), end(), new_data);
			std::destroy(begin(), end());
			if (!is_sbo()) {
				free(data); //NOLINT
			}
			m_data = new_data;
			m_capacity = m_size;
		}
	}

	//if *this will end up with either size = a, or b, this will make sure there'll be at most 1 allocation
	//if a<=sbo_size, we can avoid allocating, but if we avoid allocating, and it ends up with size=b,
	//we might allocate multiple times
	//if called with a,b where a<=sbo_size and b<=sbo_size*2 at most 1 allocation will be done	
	void reserve_wat_choose_one(int a, int b) {
		const auto [min, max] = std::minmax(a, b);
		if (min <= sbo_size) {
			if (max <= sbo_size * 2) {
				//don't reserve,
				//if we reserve, we always have 1 allocation even if we end up with min elems
				//if we don't reserve we have at most 1 allocation
			} else {
				reserve(max);
			}
		} else {
			reserve(max);
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
			return std::launder((T*)&m_storage);
		} else {
			return m_data;
		}
	}

	T* end() {
		return begin() + m_size;
	}

	const T* begin() const {
		if (is_sbo()) {
			return std::launder((T*)&m_storage);
		} else {
			return m_data;
		}
	}

	const T* end() const {
		return begin() + m_size;
	}

	const T* cbegin() const {
		if (is_sbo()) {
			return std::launder((T*)&m_storage);
		} else {
			return m_data;
		}
	}

	const T* cend() const {
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

	const T* data() const {
		return begin();
	}


	void push_back(T thing) {
		if (size() == capacity()) {
			reserve(capacity() * 2);//capacity>0 always
		}
		new(&begin()[m_size++])T(std::move(thing));
	}

	template<typename... Args>
	T& emplace_back(Args&& ...args) {
		if (size() == capacity()) {
			reserve(std::max((size_t)1, capacity() * 2));
		}
		memset(data() + size(), 0, sizeof(T));
		return *new(&begin()[m_size++])T(std::forward<Args>(args)...);
	}

	void pop_back() {
		--m_size;
		std::destroy_at(begin() + m_size);
	}

	size_t capacity() const noexcept {
		return m_capacity;
	}

	void resize(size_t new_size) {
		if (new_size < size()) {
			std::destroy(begin() + new_size, begin() + size());
			m_size = (int)new_size;
		} else if (new_size > size()) {
			reserve(new_size);
			memset(data() + size(), 0, (new_size - size())*sizeof(T));
			std::uninitialized_default_construct(begin() + size(), begin() + new_size);
			m_size = (int)new_size;
		}//else size()==new_size => do nothing
	}

	void resize(size_t new_size, const T& val) {
		if (new_size < size()) {
			std::destroy(begin() + new_size, begin() + size());
		} else if ((int)new_size > size()) {
			reserve(new_size);
			std::uninitialized_fill(begin() + size(), begin() + new_size, val);
			m_size = (int)new_size;
		}
	}

	template<typename I, typename S>
	void assign(I&& b, S&& e) {
		if constexpr (ranges::sized_sentinel_for<S, I>) {
			reserve(ranges::distance(b, e));
		}
		ranges::actions::push_back(*this, ranges::make_subrange(b, e));
	}

	iterator insert(const_iterator where_, T thing) {
		const auto where = (iterator)where_;
		push_back(std::move(thing));
		std::rotate(where, end() - 1, end());
		return where;
	}

	template<typename I, typename S>
	iterator insert(const_iterator where_, I&& b, S&& e) {
		auto where = (iterator)where_;
		const auto idx = std::distance(begin(), where);
		if constexpr (ranges::sized_sentinel_for<S, I>) {
			const auto input_size = ranges::distance(b, e);
			assert(input_size >= 0);
			if (size() + input_size > capacity()) {
				reserve(std::max(capacity() * 2, size() + input_size));
			}
			const auto new_end = std::uninitialized_copy(b, e, end());
			m_size += (int)input_size;
			std::rotate(begin() + idx, end() - input_size, end());
			return begin() + idx;
		} else {
			int input_size = 0;
			for (; b != e; ++b) {
				push_back(*b);
				++input_size;
			}
			std::rotate(begin() + idx, end() - input_size, end());
			return begin() + idx;
		}
	}

	void erase(const_iterator a, const_iterator b) {
		const auto erase_size = std::distance(a, b);
		std::shift_left((iterator)a, end(), erase_size);
		std::destroy(end() - erase_size, end());
		m_size -= (int)erase_size;
	}

	void erase(const_iterator a) {
		std::shift_left((iterator)a, end(), 1);
		std::destroy_at(end() - 1);
		--m_size;
	}

	void erase(iterator a) {
		std::shift_left((iterator)a, end(), 1);
		std::destroy_at(end() - 1);
		--m_size;
	}

	template<typename U>
	void erase(const U& a) {
		const auto it = std::remove(begin(), end(), a);
		erase(it, end());
	}

	template<typename fn>
	void erase_if(fn&& a) {
		const auto it = std::remove_if(begin(), end(), std::forward<fn>(a));
		erase(it, end());
	}

	T& operator[](size_t idx) {
		return data()[idx];
	}

	const T& operator[](size_t idx) const {
		return data()[idx];
	}

	T& at(size_t idx) {
		if (idx >= size()) {
			throw std::runtime_error("");
		}
		return data()[idx];
	}

	const T& at(size_t idx) const {
		if (idx >= size()) {
			throw std::runtime_error("");
		}
		return data()[idx];
	}

	auto rbegin() noexcept {
		return std::make_reverse_iterator(end());
	}

	auto rend() noexcept {
		return std::make_reverse_iterator(begin());
	}

	auto rbegin() const noexcept {
		return std::make_reverse_iterator(end());
	}

	auto rend() const noexcept {
		return std::make_reverse_iterator(begin());
	}

	auto crbegin() const noexcept {
		return std::make_reverse_iterator(end());
	}

	auto crend() const noexcept {
		return std::make_reverse_iterator(begin());
	}

	bool operator==(const sbo_vector& other) const noexcept {
		return size() == other.size() && std::equal(begin(), end(), other.begin());
	}

	template<typename...Args>
	iterator emplace(const_iterator where, Args&&... args) {
		T thing = T(std::forward<Args>(args)...);
		return insert(where, std::move(thing));
	}

	T& front() {
		return *data();
	}

	const T& front() const {
		return *data();
	}

	T& back() {
		return data()[size() - 1];
	}

	const T& back() const {
		return data()[size() - 1];
	}

private:

	union {
		alignas(T) std::array<std::byte, sbo_size * sizeof(T)> m_storage = {};
		T* m_data;
	};
	//these are ints to save space? ;-;
	int m_capacity = sbo_size;
	int m_size = 0;

	void die() {
		clear();
		if (!is_sbo()) {
			free(m_data); //NOLINT
			m_capacity = sbo_size;
		}
	}
};
