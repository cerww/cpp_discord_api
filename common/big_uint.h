#pragma once
#include <cstdint>
#include <compare>
#include <string_view>
#include <functional>
#include <span>
#include <charconv>
#include <fmt/format.h>
#include <range/v3/all.hpp>
#include "sbo_vector.h"
#include <iostream>


struct big_uint {
	big_uint() = default;

	big_uint(uint64_t a) {
		if (a) {
			const auto w2 = (uint32_t)(a >> 32);
			if (w2) {
				m_data.resize(2);
				m_data[1] = w2;
				m_data[0] = a & 0xffffffff;
			} else {
				m_data.push_back((uint32_t)a);
			}
		}
	}

	big_uint(uint32_t a) {
		if (a) {
			m_data.push_back(a);
		}
	}

	big_uint(uint16_t a) {
		if (a) {
			m_data.push_back(a);
		}
	}

	big_uint(uint8_t a) {
		if (a) {
			m_data.push_back(a);
		}
	}

	big_uint(int64_t a) {
		if (a) {
			const auto w2 = (uint32_t)(a >> 32);
			if (w2) {
				m_data.resize(2);
				m_data[1] = w2;
				m_data[0] = a & 0xffffffff;
			} else {
				m_data.push_back((uint32_t)a);
			}
		}
	}

	big_uint(int32_t a) {
		if (a) {
			m_data.push_back((uint32_t&)a);
		}
	}

	big_uint(int16_t a) {
		if (a) {
			m_data.push_back((uint16_t&)a);
		}
	}

	big_uint(int8_t a) {
		if (a) {
			m_data.push_back((uint8_t&)a);
		}
	}

	// big_uint(std::vector<uint32_t> data_t) :
	// 	m_data(std::move(data_t)) {
	// 	trim_trailing_zeros();
	// }
	
	big_uint(const std::vector<uint32_t>& data_t) :
		m_data(std::move(data_t) | ranges::to<sbo_vector<uint32_t,4>>()) {
		trim_trailing_zeros();
	}
	
	big_uint(sbo_vector<uint32_t,4> data_t) :
		m_data(std::move(data_t)) {
		trim_trailing_zeros();
	}

	explicit big_uint(std::string_view s) {
		//;-;
		//assume base 10
		//assume input is correct

		//s.remove_suffix(std::distance(s.rbegin(), std::find_if_not(s.rbegin(), s.rend(), [](char c) {return c == '0'; })));
		const auto idx_of_first_non_zero = s.find_first_not_of('0');
		if (idx_of_first_non_zero == std::string_view::npos) {
			return;
		}
		s.remove_prefix(idx_of_first_non_zero);
		if (s.empty()) {
			return;
		}

		const int digits = (int)log10(s.size() + 1) + 1;
		m_data.reserve(1 + (size_t)digits / 8);
		while (!s.empty()) {
			const int digits_to_take = (int)std::min(9ull, s.size());
			uint32_t r = 0;
			const auto result = std::from_chars(s.data(), s.data() + digits_to_take, r);
			if (result.ptr == s.data()) {
				return;
			}
			*this *= (uint32_t)std::pow(10, digits_to_take);
			*this += r;
			s.remove_prefix(digits_to_take);
		}

		// for(auto digits_chunk: s | ranges::views::chunk(9)) {
		// 	const int chunk_len = (int)std::min(9ull, ranges::distance(digits_chunk));
		// 	uint32_t r;
		// 	const auto result = std::from_chars(digits_chunk.data(), digits_chunk.data() + chunk_len, r);
		// 	if (result.ptr == s.data()) {
		// 		return;
		// 	}
		// 	*this *= (int)std::pow(10, chunk_len);
		// 	*this += r;
		// }
	}

	explicit big_uint(std::string_view s, int base) {
		//;-;
		//assume input is correct

		const auto idx_of_first_non_zero = s.find_first_not_of('0');
		if (idx_of_first_non_zero == std::string_view::npos) {
			return;
		}
		s.remove_prefix(idx_of_first_non_zero);
		if (s.empty()) {
			return;
		}

		const int digits = (int)log10(s.size() + 1);
		m_data.reserve(digits / 8);
		while (!s.empty()) {
			const int digits_to_take = (int)std::min(4ull, s.size());
			uint32_t r;
			const auto result = std::from_chars(s.data(), s.data() + digits_to_take, r, base);
			if (result.ptr == s.data()) {
				return;
			}
			*this *= (int)std::pow(base, digits_to_take);
			*this += r;
			s.remove_prefix(digits_to_take);
		}

		// for(auto digits_chunk: s | views::chunk(9)) {
		// 	const int chunk_len = (int)std::min(9ull, ranges::distance(digits_chunk));
		// 	uint32_t r;
		// 	const auto result = std::from_chars(digits_chunk.data(), digits_chunk.data() + chunk_len, r);
		// 	if (result.ptr == s.data()) {
		// 		return;
		// 	}
		// 	*this *= (int)std::pow(base, chunk_len);
		// 	*this += r;
		// }

		// digits_count_n = logn(x+1);
		// digits_count_b = logb(x+1);
		// logb(n^digits_count_n);
	}

	friend big_uint operator+(const big_uint& a, const big_uint& b) {
		const auto size = std::max(a.data().size(), b.data().size());

		const auto thing_a = [&](int idx)->uint32_t {
			if (idx >= a.data().size()) {
				return 0;
			} else {
				return a.data()[idx];
			}
		};

		const auto thing_b = [&](int idx)->uint32_t {
			if (idx >= b.data().size()) {
				return 0;
			} else {
				return b.data()[idx];
			}
		};

		std::vector<uint32_t> ret_array;
		ret_array.reserve(size + 1);
		ret_array.resize(size);
		uint32_t carry = 0;
		for (int i = 0; i < size; ++i) {
			const uint64_t t = (uint64_t)thing_a(i) + (uint64_t)thing_b(i) + carry;
			ret_array[i] = (uint32_t)((t) & 0xFFFFFFFF);
			carry = t >> 32;
		}
		if (carry) {
			ret_array.push_back(carry);
		}
		return big_uint(std::move(ret_array)).trim_trailing_zeros();
	}

	friend big_uint operator+(const big_uint& a, int b) {
		return a + big_uint(b);
	}

	friend big_uint operator-(big_uint a, const big_uint& b) {
		const auto size = std::max(a.data().size(), b.data().size());
		a.m_data.resize(size);

		const auto thing_a = [&](int idx)->uint32_t {
			return a.data()[idx];
		};

		const auto thing_b = [&](int idx)->uint32_t {
			if (idx >= b.data().size()) {
				return 0;
			} else {
				return b.data()[idx];
			}
		};

		int amount_carried = 0;
		for (int i = 0; i < size; ++i) {
			if ((int64_t)thing_b(i) > (int64_t)thing_a(i) - amount_carried) {//need to borrow
				a.m_data[i] = std::numeric_limits<uint32_t>::max() - thing_b(i) - amount_carried + thing_a(i) + 1;
				amount_carried = 1;
			} else {
				a.m_data[i] = thing_a(i) - thing_b(i) - amount_carried;
				amount_carried = 0;
			}
		}

		if (amount_carried) {
			//;-;
		}
		return a.trim_trailing_zeros();
	}

	friend big_uint& operator-=(big_uint& a, const big_uint& b) {
		return a = std::move(a) - b;
	}

	friend big_uint operator&(big_uint a, const big_uint& b) {
		const auto size = std::max(a.data().size(), b.data().size());

		const auto thing_b = [&](int idx)->uint32_t {
			if (idx >= b.data().size()) {
				return 0;
			} else {
				return b.data()[idx];
			}
		};

		a.m_data.resize(size);

		for (int i = 0; i < size; ++i) {
			a.m_data[i] &= thing_b(i);
		}
		a.trim_trailing_zeros();
		return a;
	}

	friend big_uint operator|(big_uint a, const big_uint& b) {
		const auto size = std::max(a.data().size(), b.data().size());

		const auto thing_b = [&](int idx)->uint32_t {
			if (idx >= b.data().size()) {
				return 0;
			} else {
				return b.data()[idx];
			}
		};

		a.m_data.resize(size);

		for (int i = 0; i < size; ++i) {
			a.m_data[i] |= thing_b(i);
		}

		a.trim_trailing_zeros();
		return a;
	}

	std::span<const uint32_t> data() const noexcept {
		return m_data;
	}

	bool operator==(const big_uint&) const = default;

	friend big_uint& operator&=(big_uint& a, const big_uint& b) {
		return a = std::move(a) & b;
	}

	friend big_uint& operator|=(big_uint& a, const big_uint& b) {
		return a = std::move(a) | b;
	}

	friend big_uint& operator+=(big_uint& a, const big_uint& b) {
		const auto size = std::max(a.data().size(), b.data().size());

		const auto thing_a = [&](int idx)->uint64_t {
			if (idx >= a.data().size()) {
				return 0;
			} else {
				return a.data()[idx];
			}
		};

		const auto thing_b = [&](int idx)->uint64_t {
			if (idx >= b.data().size()) {
				return 0;
			} else {
				return b.data()[idx];
			}
		};

		a.m_data.reserve(size + 1);
		a.m_data.resize(size);

		uint32_t carry = 0;
		for (int i = 0; i < size; ++i) {
			const uint64_t t = (uint64_t)thing_a(i) + (uint64_t)thing_b(i) + carry;
			a.m_data[i] = (uint32_t)(t & 0xFFFFFFFF);
			carry = t >> 32;
		}
		if (carry) {
			a.m_data.push_back(carry);
		}
		return a.trim_trailing_zeros();
	}

	friend big_uint& operator*=(big_uint& a, uint32_t other) {
		return a = a * other;
	}

	friend big_uint& operator*=(big_uint& a, const big_uint& other) {
		
		return a = a * other;
	}

	friend big_uint& operator*=(big_uint&& a, const big_uint& other) {
		return a = a * other;
	}

	friend big_uint operator*(const big_uint& a, const big_uint& b) {
		if (a == 0 || b == 0) {
			return 0;
		}
		big_uint return_value = 0;
		for (size_t j = 0; j < b.m_data.size(); ++j) {
			const auto digit_b = b.m_data[j];
			std::vector<uint32_t> rt;
			rt.reserve(a.m_data.size() + 1 + j);
			rt.resize(a.m_data.size() + j);

			uint32_t carry = 0;
			for (size_t i = 0; i < a.m_data.size(); ++i) {
				const auto digit = a.m_data[i];
				const uint64_t t = (uint64_t)digit * (uint64_t)digit_b + carry;
				rt[i + j] = (uint32_t)(t & 0xFFFFFFFF);
				carry = (uint32_t)(t >> 32);
			}

			if (carry) {
				rt.push_back(carry);
			}

			return_value += big_uint(std::move(rt)).trim_trailing_zeros();
			return_value.trim_trailing_zeros();
		}
		return_value.trim_trailing_zeros();
		return return_value;
	}

	friend big_uint operator*(const big_uint& a, uint32_t b) {
		if (a == 0 || b == 0) {
			return 0;
		}
		big_uint ret = 0;
		ret.m_data.reserve(a.m_data.size() + 1);
		uint64_t carry = 0;
		for (auto digit : a.m_data) {
			const uint64_t t = (uint64_t)digit * (uint64_t)b + carry;
			ret.m_data.push_back(t & 0xffffffff);
			carry = t >> 32;
		}
		if (carry) {
			ret.m_data.push_back((uint32_t)carry);
		}
		return ret;
	}

	friend big_uint operator*(const big_uint& a, int32_t b) {
		if (a == 0 || b == 0) {
			return 0;
		}
		big_uint ret = 0;
		ret.m_data.reserve(a.m_data.size() + 1);
		uint64_t carry = 0;
		for (auto digit : a.m_data) {
			const uint64_t t = (uint64_t)digit * (uint64_t)b + carry;
			ret.m_data.push_back(t & 0xffffffff);
			carry = t >> 32;
		}
		if (carry) {
			ret.m_data.push_back((uint32_t)carry);
		}
		return ret;
	}

	friend big_uint operator*(const big_uint& a, uint64_t b) {
		return a * big_uint(b);
		// if (a == 0 || b == 0) {
		// 	return 0;
		// }
		//
		// const auto w1 = (uint32_t)(b & 0xffffffff);
		// const auto w2 = (uint32_t)(b >> 32);
		//
		// big_uint r1 = a * w1;
		// big_uint r2 = a * w2;

		//return ret;
	}

	friend big_uint bitwise_and_but_missing_bits_are_1(const big_uint& a, const big_uint& b) {
		const auto size = std::max(a.data().size(), b.data().size());

		const auto thing_a = [&](int idx)->uint32_t {
			if (idx >= a.data().size()) {
				return 0xFFFFFFFF;
			} else {
				return a.data()[idx];
			}
		};

		const auto thing_b = [&](int idx)->uint32_t {
			if (idx >= b.data().size()) {
				return 0xFFFFFFFF;
			} else {
				return b.data()[idx];
			}
		};

		std::vector<uint32_t> ret_array;
		ret_array.resize(size);

		for (int i = 0; i < size; ++i) {
			ret_array[i] = thing_a(i) & thing_b(i);
		}

		return big_uint(std::move(ret_array)).trim_trailing_zeros();
	}

	friend big_uint bitwise_and_but_missing_bits_are_1(big_uint&& a, const big_uint& b) {
		const auto size = std::max(a.data().size(), b.data().size());
			
		const auto thing_b = [&](int idx)->uint32_t {
			if (idx >= b.data().size()) {
				return 0xFFFFFFFF;
			} else {
				return b.data()[idx];
			}
		};

		a.m_data.resize(size, 0xFFFFFFFF);

		for (int i = 0; i < size; ++i) {
			a.m_data[i] &= thing_b(i);
		}

		return std::move(a).trim_trailing_zeros();
	}

	friend big_uint operator~(const big_uint& a) {
		return big_uint(a.m_data | ranges::views::transform([](uint32_t b) { return ~b; }) | ranges::to<std::vector>());
	}

	friend big_uint operator~(big_uint&& a) {
		//return big_int(a.m_data | ranges::views::transform([](uint32_t b) {return ~b; }) | ranges::to<std::vector>());
		//using namespace ranges::actions;
		a.m_data |= ranges::actions::transform([](uint32_t b) { return ~b; });
		a.trim_trailing_zeros();
		return std::move(a);
	}

	bool operator==(uint32_t other) const noexcept {
		if (other == 0 && m_data.empty()) {
			return true;
		}
		return m_data.size() == 1 && m_data.front() == other;
	}

	bool operator==(int32_t other) const noexcept {
		if (other == 0 && m_data.empty()) {
			return true;
		}
		return other >= 0 && m_data.size() == 1 && m_data.front() == (uint32_t)other;
	}

	bool operator==(int16_t other) const noexcept {
		if (other == 0 && m_data.empty()) {
			return true;
		}
		return other >= 0 && m_data.size() == 1 && m_data.front() == (uint32_t)other;
	}

	bool operator==(int8_t other) const noexcept {
		if (other == 0 && m_data.empty()) {
			return true;
		}
		return other >= 0 && m_data.size() == 1 && m_data.front() == (uint32_t)other;
	}


	bool operator==(uint16_t other) const noexcept {
		if (other == 0 && m_data.empty()) {
			return true;
		}
		return other >= 0 && m_data.size() == 1 && m_data.front() == (uint32_t)other;
	}

	bool operator==(uint8_t other) const noexcept {
		if (other == 0 && m_data.empty()) {
			return true;
		}
		return m_data.size() == 1 && m_data.front() == (uint32_t)other;
	}


	bool operator==(uint64_t other) const noexcept {
		if (other == 0 && m_data.empty()) {
			return true;
		}
		const auto w2 = other >> 32;
		if (w2) {
			return m_data.size() == 2 && m_data[1] == w2 && (m_data[0] == (uint32_t)(other & 0xffffffff));
		} else {
			return m_data.size() == 1 && m_data.front() == (uint32_t)other;
		}
	}

	bool operator==(int64_t other) const noexcept {
		if (other == 0 && m_data.empty()) {
			return true;
		}
		if (other < 0) {
			return false;
		}
		return *this == (uint64_t)other;
	}

private:
	big_uint& trim_trailing_zeros() {
		const auto it = std::find_if(m_data.rbegin(), m_data.rend(), [](uint32_t a) { return a != 0; });
		m_data.erase(it.base(), m_data.end());
		return *this;
	}

	//std::vector<uint32_t> m_data;
	sbo_vector<uint32_t, 4> m_data;
};

struct big_uint_to_string_accumulator {

	big_uint_to_string_accumulator() = default;

	explicit big_uint_to_string_accumulator(std::string d):
		m_data(std::move(d)) {}

	big_uint_to_string_accumulator& operator+=(const uint32_t n) {
		uint64_t carry = n;
		for (char& c : m_data) {
			const auto r = c + carry;
			c = (char)(r % 10);
			carry = r / 10;
		}
		if (carry) {
			m_data += std::to_string(carry) | ranges::actions::transform([](char c) { return c - '0'; }) | ranges::actions::reverse;
		}
		return *this;
	}

	//52*5213
	//25
	//2*5213 = 10426
	//6
	//carry = 1042
	//5
	//26065 + 1042
	//27107
	//7
	//2710
	//0172
	//670172
	//271076

	big_uint_to_string_accumulator& operator*=(uint64_t n) {
		big_uint_to_string_accumulator ret;

		for (int i = 0; i < (int)m_data.size(); ++i) {
			ret += big_uint_to_string_accumulator(
				std::to_string(m_data[i] * n)
				| ranges::actions::transform([](char c) { return c - '0'; })
				| ranges::actions::reverse
			).multiply_by_10_to_pow(i);
		}

		return *this = std::move(ret);
	}

	big_uint_to_string_accumulator& operator+=(const big_uint_to_string_accumulator& n) {
		if (n.m_data.size() > m_data.size()) {
			m_data.resize(n.m_data.size(), 0);
		}

		const auto other_n = [&](int idx) {
			if (idx >= (int)n.m_data.size()) {
				return 0;
			}
			return (int)n.m_data[idx];
		};

		char carry = 0;
		for (int i = 0; i < (int)m_data.size(); ++i) {
			m_data[i] += other_n(i) + carry;
			carry = (char)(m_data[i] / (char)10);
			m_data[i] %= 10;
		}

		if (carry) {
			m_data += std::to_string((uint64_t)carry) | ranges::actions::transform([](char c) { return c - '0'; }) | ranges::actions::reverse;
		}
		return *this;
	}

	big_uint_to_string_accumulator& multiply_by_10_to_pow(int exp) {
		m_data.resize(m_data.size() + exp);
		std::shift_right(m_data.begin(), m_data.end(), exp);
		std::fill(m_data.begin(), m_data.begin() + exp, 0);
		return *this;
	}

	void reserve_before(size_t n) {
		m_data.reserve(n);
	}

	std::string get_final_result() {
		return std::move(m_data |= ranges::actions::transform([](char c) { return c + '0'; }) | ranges::actions::reverse);
	}

private:
	std::string m_data;
};

inline std::string to_string(const big_uint& value) {
	if (value == 0) {
		return "0";
	}

	std::cout << "aaaaa" << std::endl;
	big_uint_to_string_accumulator acc;
	for (auto n : value.data() | ranges::views::reverse) {
		std::cout << n << std::endl;
		acc *= (uint64_t)std::numeric_limits<uint32_t>::max() + 1;
		acc += n;
	}
	std::cout << "bbbbb" << std::endl;
	return acc.get_final_result();
}

template<typename Char>
struct fmt::formatter<big_uint,Char>:formatter<uint64_t,Char> {
	
	template <typename FormatContext>
	auto format(const big_uint& val, FormatContext& ctx) { 
		return formatter<uint64_t>::format(to_string(val), ctx);
	}
};

