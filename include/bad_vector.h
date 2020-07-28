#pragma once
#include <utility>
#include <algorithm>
#include <memory>
#include <vector>
#include <stdexcept>


//bad vector where size == cap, saves 8 bytes of memory  ;-;
template<typename T>
struct bad_vector {
	bad_vector() = default;

	template<typename it, typename sent>
	bad_vector(it&& begin, sent&& end) {
		m_size = std::distance(begin, end);
		m_data = (T*)malloc(m_size * sizeof(T));
		std::uninitialized_copy(begin, end, m_data);
	}

	template<typename range> requires !std::is_same_v<std::decay_t<range>, bad_vector<T>>//to not hide copy constructors
	explicit bad_vector(range&& a):
		bad_vector(a.begin(), a.end()) { }

	bad_vector(const bad_vector& other):bad_vector(other.begin(),other.end()){
		
	}

	bad_vector(bad_vector&& other) noexcept:
		m_data(std::exchange(other.m_data,nullptr)),
		m_size(std::exchange(other.m_size,0)){

	}

	bad_vector& operator=(bad_vector&& other) noexcept{
		if(this == &other) {
			return *this;
		}
		m_data = (std::exchange(other.m_data, nullptr));
		m_size = (std::exchange(other.m_size, 0));
		return *this;
	}

	bad_vector& operator=(const bad_vector& other) {
		if(this == &other) {
			return *this;
		}
		m_size = other.size();
		m_data = (T*)malloc(m_size * sizeof(T));
		std::uninitialized_copy(other.begin(), other.end(),m_data);
		return *this;
	}	

	~bad_vector() {
		std::destroy(m_data, m_data + m_size);
		free(m_data);
	}

	void resize(const size_t new_size) {
		if (new_size == m_size) {
			return;
		} else if (new_size > m_size) {
			const auto new_data = (T*)malloc(new_size * sizeof(T));

			std::uninitialized_move(m_data, m_data + m_size, new_data);
			std::uninitialized_default_construct(new_data + m_size, new_data + new_size);
			std::destroy(m_data, m_data + m_size);
			m_data = new_data;
			m_size = new_size;
		} else {//new_size<m_size
			const auto new_data = (T*)malloc(new_size * sizeof(T));
			std::uninitialized_move(m_data, m_data + new_size, new_data);
			std::destroy(m_data, m_data + m_size);
			m_data = new_data;
			m_size = new_size;
		}
	}
	
	void resize(const size_t new_size,T fill_value) {
		if (new_size == m_size) {
			return;
		}
		else if (new_size > m_size) {
			const auto new_data = (T*)malloc(new_size * sizeof(T));

			std::uninitialized_move(m_data, m_data + m_size, new_data);
			std::uninitialized_fill(new_data + m_size, new_data + new_size, fill_value);
			std::destroy(m_data, m_data + m_size);
			m_data = new_data;
			m_size = new_size;
		}
		else {//new_size<m_size
			const auto new_data = (T*)malloc(new_size * sizeof(T));
			std::uninitialized_move(m_data, m_data + new_size, new_data);
			std::destroy(m_data, m_data + m_size);
			m_data = new_data;
			m_size = new_size;
		}
	}

	template<typename range>
	void assign(range&& r) {
		//requires sized_range
		clear();
		const auto size = r.size();
		m_data = malloc(size * sizeof(T));
		m_size = size;
		std::uninitialized_copy(r.begin(), r.end(), m_data);		
	}
	

	//no push_back,emplace_back,erase,reserve

	T& operator[](const size_t idx) {
		return m_data[idx];
	}

	const T& operator[](const size_t idx) const {
		return m_data[idx];
	}

	const T& at(const size_t idx) const {
		if (idx >= m_size) {
			throw std::out_of_range("");
		}
		return m_data[idx];
	}

	T& at(const size_t idx) {
		if (idx >= m_size) {
			throw std::out_of_range("");
		}
		return m_data[idx];
	}

	T* begin() {
		return m_data;
	}
	
	T* end() {
		return m_data + m_size;
	}

	const T* begin() const {
		return m_data;
	}
	
	const T* end()const {
		return m_data + m_size;
	}

	T* data() {
		return m_data;
	}

	const T* data() const {
		return m_data;
	}

	size_t size()const noexcept {
		return m_size;
	}

	size_t capacity()const noexcept {
		return m_size;
	}

	size_t max_size()const noexcept {
		return std::numeric_limits<size_t>::max();
	}

	void shrink_to_fit() {
		//do nothing
	}

	void clear() {
		std::destroy(m_data, m_data + m_size);
		free(m_data);
		m_data = nullptr;
		m_size = 0;
	}

	bool empty()const noexcept {
		return m_size == 0;
	}
	
private:
	T* m_data = nullptr;
	size_t m_size = 0;//no benifit of using int instead cuz of alignment
};
