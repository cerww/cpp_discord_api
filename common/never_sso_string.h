#pragma once
#include <memory>
#include <string>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <string_view>
#include <nlohmann/json.hpp>

//no null terminator
struct never_sbo_string {

	constexpr static size_t npos = -1;	
	
	never_sbo_string() = default;

	never_sbo_string(const never_sbo_string& other) {
		if (other.size()) {
			m_data = (char*)malloc(other.size());  // NOLINT
			std::copy(other.begin(), other.end(), m_data);
			m_capacity = m_size = (int)other.size();
		}
	}

	never_sbo_string(const std::string& s):never_sbo_string(s.data(),s.size()) {
		
	}

	explicit never_sbo_string(std::string_view s) :never_sbo_string(s.data(), s.size()) {

	}

	never_sbo_string(const char* c_str) {
		const auto len = strlen(c_str);
		m_capacity = m_size = (int)len;
		m_data = (char*)malloc(len);  // NOLINT
		std::copy(c_str, c_str + m_size, m_data);
	}

	never_sbo_string(const char* c_str, size_t len) {
		m_capacity = m_size = (int)len;
		m_data = (char*)malloc(len);  // NOLINT
		std::copy(c_str, c_str + m_size, m_data);
	}

	never_sbo_string(never_sbo_string&& other) noexcept :
		m_data(std::exchange(other.m_data, nullptr)),
		m_size(std::exchange(other.m_size, 0)),
		m_capacity(std::exchange(other.m_capacity, 0)) { }

	never_sbo_string& operator=(const never_sbo_string& other) {
		if (&other == this) {
			return *this;
		}
		if (other.size()) {
			if (m_data) {
				free(m_data);  // NOLINT(cppcoreguidelines-no-malloc)
			}
			m_data = (char*)malloc(other.size());  // NOLINT
			std::copy(other.begin(), other.end(), m_data);
			m_capacity = m_size = (int)other.size();
		}
		return *this;
	}

	never_sbo_string& operator=(never_sbo_string&& other) noexcept {
		m_data = std::exchange(other.m_data, nullptr);
		m_size = std::exchange(other.m_size, 0);
		m_capacity = std::exchange(other.m_capacity, 0);
		return *this;
	}

	~never_sbo_string() {
		if (m_data) {
			free(m_data);  // NOLINT
		}
	}

	void resize(size_t n) {
		if (n <= (int)m_size) {  // NOLINT
			m_size = (int)n;
		}
		else {
			reserve(n);
			m_size = (int)n;
		}
	}

	void resize(size_t n, char fill) {
		if (n <= (int)m_size) {  // NOLINT
			m_size = (int)n;
		}
		else {
			reserve(n);
			std::fill(m_data + m_size, m_data + n, fill);
			m_size = (int)n;
		}
	}

	void reserve(size_t new_cap) {
		if (new_cap <= (int)m_capacity) {  // NOLINT
			return;
		}
		else {
			if (m_capacity == 0) {
				m_data = (char*)malloc(new_cap);  // NOLINT
				m_capacity = (int)new_cap;
			}
			char* new_data = (char*)malloc(new_cap);  // NOLINT
			std::copy(m_data, m_data + size(), new_data);
			free(m_data);  // NOLINT
			m_data = new_data;
			m_capacity = (int)new_cap;
		}
	}

	size_t size() const noexcept {
		return m_size;
	}

	size_t capacity() const noexcept {
		return m_capacity;
	}

	char* begin() {
		return m_data;
	}

	char* end() {
		return m_data + m_size;
	}

	const char* begin() const noexcept {
		return m_data;
	}

	const char* end() const noexcept {
		return m_data + m_size;
	}

	char& operator[](size_t idx) {
		return m_data[idx];
	}

	const char& operator[](size_t idx) const {
		return m_data[idx];
	}


	char& at(size_t idx) {
		if (idx >= m_size) {  // NOLINT
			throw std::out_of_range("");
		}
		return m_data[idx];
	}

	const char& at(size_t idx) const {
		if (idx >= m_size) {  // NOLINT
			throw std::out_of_range("");
		}
		return m_data[idx];
	}

	char* data() {
		return m_data;
	}

	const char* data() const {
		return m_data;
	}

	size_t length() const noexcept {
		return m_size;
	}

	bool empty()const noexcept {
		return m_size == 0;
	}

	void shrink_to_fit() {
		if(m_size == m_capacity) {
			return;
		}
		if(empty()) {
			free(m_data);  // NOLINT
			m_size = m_capacity = 0;
		}
		char* new_data = (char*)malloc(m_size);  // NOLINT
		std::copy(m_data, m_data + m_size, new_data);
		m_capacity = m_size;
		free(m_data);  // NOLINT
		m_data = new_data;
	}
	
	operator std::string_view() const{
		return std::string_view(m_data, m_size);
	}

	never_sbo_string& operator+=(const std::string& other){
		reserve(m_size + other.size());
		std::copy(other.begin(), other.end(),m_data+m_size);
		m_size += (int)other.size();
		return *this;
	}

	never_sbo_string& operator+=(const never_sbo_string& other) {
		reserve(m_size + other.size());
		std::copy(other.begin(), other.end(), m_data + m_size);
		m_size += (int)other.size();
		return *this;		
	}

	never_sbo_string operator+(const std::string& s)const {
		return never_sbo_string(*this) += s;
	}

	never_sbo_string operator+(const never_sbo_string& s)const {
		return never_sbo_string(*this) += s;
	}
	
private:
	char* m_data = nullptr;
	int m_size = 0;
	int m_capacity = 0;
};




