#pragma once
#include <algorithm>
#include <string>
#include <array>

struct sbo_string2 {
	constexpr static int sbo_size = 23;	
	
	sbo_string2() {
		std::fill(m_sbo_data.begin(), m_sbo_data.end() - 1, (char)0);
		m_sbo_data[23] = 23;
	}

	size_t capacity() const noexcept{
		if(is_sbo()) {
			return 23;
		}else{
			return m_capacity & 0x7FFFFFFFFFFFFFFF;//requires little endian
			//return m_capacity & 0xFFFFFFFFFFFFFF7F for big endian
		}
	}

	void push_back(char c) {
		if(is_sbo()) {
			if(m_sbo_data.back() == 0) {
				reserve(32);
			}else {
				m_sbo_data[23 - m_sbo_data.back()--] = c;
			}
		}else {
			if(m_size == m_capacity) {
				reserve(m_capacity * 2);
			}
			m_data[m_size++] = c;
		}
	}

	void reserve(size_t new_size) {
		const size_t actual_new_size = ((new_size - 1) / 16) * 16 + 16;
		char* new_data = (char*)malloc(actual_new_size);
		std::copy(begin(), end(), new_data);
		if(is_sbo()) {
			
		} else {
			free(m_data);
		}
		m_capacity = actual_new_size;
	}

	char* begin() {
		if(is_sbo()) {
			return m_sbo_data.data();
		}
		else {
			return m_data;
		}
	}

	char* end() {
		if (is_sbo()) {
			return &m_sbo_data[23 - m_sbo_data.back()];
		}else {
			return m_data + m_size;
		}		
	}

	const char* begin() const {
		if (is_sbo()) {
			return m_sbo_data.data();
		}
		else {
			return m_data;
		}
	}

	const char* end()const {
		if (is_sbo()) {
			return &m_sbo_data[23 - m_sbo_data.back()];
		}
		else {
			return m_data + m_size;
		}
	}
	
	
private:
	
	union {
		std::array<char, 24> m_sbo_data;
		struct {
			char* m_data = nullptr;
			size_t m_size;
			size_t m_capacity;//little endian
		};
	};
	//char m_sbo_flag_or_remaining_sbo_space = 23;

	bool is_sbo() const noexcept{
		return !(m_sbo_data.back() & 255);
	}
};
