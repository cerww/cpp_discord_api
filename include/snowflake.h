#pragma once
#include <nlohmann/json.hpp>
#include <utility>
#include <type_traits>
#include "../common/ref_stable_map.h"
#include <charconv>
#include "../common/iterator_facade.h"
#include <compare>
#include <fmt/core.h>
#include <fmt/format.h>
#include "../common/ref_stable2.h"

struct snowflake {
	constexpr bool operator==(const snowflake& other) const noexcept = default;
	//constexpr std::strong_ordering operator<=>(const snowflake& other) const noexcept = default;

	constexpr snowflake() = default;

	constexpr snowflake& operator=(snowflake&& other) noexcept {
		std::swap(val,other.val);
		return *this;
	}

	constexpr snowflake& operator=(const snowflake& other) = default;

	constexpr snowflake(snowflake&& other) noexcept :
		val(std::exchange(other.val, 0)) {}

	constexpr snowflake(const snowflake& other) = default;
	
	~snowflake() = default;
	
	constexpr explicit snowflake(const uint64_t a):	
		val(a) {}

	uint64_t val = 0;//no reason to be private?

	uint64_t as_int()const noexcept {
		return val;
	}
};

/*
constexpr bool operator!=(const snowflake& a, const snowflake& b) {
	return a.val != b.val;
}

constexpr bool operator<(const snowflake& a, const snowflake& b) {
	return a.val < b.val;
}

constexpr bool operator>(const snowflake& a, const snowflake& b) {
	return a.val > b.val;
}

constexpr bool operator<=(const snowflake& a, const snowflake& b) {
	return a.val <= b.val;
}

constexpr bool operator>=(const snowflake& a, const snowflake& b) {
	return a.val >= b.val;
}
*/

inline void to_json(nlohmann::json& json, const snowflake& wawt) {
	char buffer[20] = {};
	const auto r = std::to_chars(buffer, buffer + 20, wawt.val);
	json = std::string(buffer, r.ptr);
}

inline void from_json(const nlohmann::json& in, snowflake& out) {
	if (in.is_null())
		return;
	const auto& str = in.get_ref<const std::string&>();
	std::from_chars(str.data(), str.data() + str.size(), out.val);
}


namespace std {
	template<>
	struct hash<snowflake> {
		size_t operator()(snowflake s) const noexcept {
			return std::hash<size_t>()(s.val);
		}
	};
}

/*
template<typename T, typename = void>
struct has_id :std::false_type {};

template<typename T>
struct has_id<T, std::void_t<decltype(std::declval<T>().id())>> :std::is_same<decltype(std::declval<T>().id()), snowflake> {};

template<typename T>
static constexpr bool has_id_v = has_id<T>::value;
*/

struct id_equal_to {
	explicit id_equal_to(snowflake i) :
		id(std::move(i)) {}

	template<typename T>
	constexpr bool operator()(T&& thing) {
		return thing.id() == id;
	}

	const snowflake id;
};

//this is like a map_view ;-;, i should just make one, then alias this
template<typename T>
struct discord_obj_map {
	discord_obj_map() = default;

	discord_obj_map(const ref_stable_map<snowflake, T>& data):
		m_data(&data) {}

	template<typename it>
	struct cursor {
		cursor() = default;

		cursor(it i) :
			m_it(i) {}

		decltype(auto) read() const noexcept {
			return *m_it;
		}

		void next() noexcept {
			++m_it;
		}

		bool operator==(const cursor& other) const {
			return m_it == other.m_it;
		}

		it m_it;
	};

	using iterator = iterator_facade<cursor<typename ref_stable_map<snowflake, T>::const_iterator>>;
	using const_iterator = iterator;

	const T& operator[](snowflake s) const {
		return m_data->at(s);
	}

	const T& at(snowflake s) const {
		return m_data->at(s);
	}

	iterator begin() const noexcept {
		return m_data->begin();
	}

	iterator end() const noexcept {
		return m_data->end();
	}

	const_iterator cbegin() const noexcept {
		return m_data->begin();
	}

	const_iterator cend() const noexcept {
		return m_data->end();
	}

	iterator find(snowflake s) const noexcept {
		return m_data->find(s);
	}

	bool contains(snowflake s) const noexcept {
		return m_data->find(s) != m_data->end();
	}

	size_t size() const noexcept {
		return m_data->size();
	}

	bool empty() const noexcept {
		return size() == 0;
	}

private:
	const ref_stable_map<snowflake, T>* m_data;
};

template<typename T>
struct discord_obj_map2 {
	discord_obj_map2() = default;

	discord_obj_map2(const ref_stable_map2<snowflake, T>& data) :
		m_data(&data) {}

	template<typename it>
	struct cursor {
		cursor() = default;

		cursor(it i) :
			m_it(i) {}

		decltype(auto) read() const noexcept {
			return *m_it;
		}

		void next() noexcept {
			++m_it;
		}

		bool operator==(const cursor& other) const {
			return m_it == other.m_it;
		}

		it m_it;
	};

	using iterator = iterator_facade<cursor<typename ref_stable_map2<snowflake, T>::const_iterator>>;
	using const_iterator = iterator;

	const T& operator[](snowflake s) const {
		return m_data->at(s);
	}

	const T& at(snowflake s) const {
		return m_data->at(s);
	}

	iterator begin() const noexcept {
		return m_data->begin();
	}

	iterator end() const noexcept {
		return m_data->end();
	}

	const_iterator cbegin() const noexcept {
		return m_data->begin();
	}

	const_iterator cend() const noexcept {
		return m_data->end();
	}

	iterator find(snowflake s) const noexcept {
		return m_data->find(s);
	}

	bool contains(snowflake s) const noexcept {
		return m_data->find(s) != m_data->end();
	}

	size_t size() const noexcept {
		return m_data->size();
	}

	bool empty() const noexcept {
		return size() == 0;
	}

private:
	const ref_stable_map2<snowflake, T>* m_data;
};

static inline constexpr auto transform_to_pair = [](auto&& a, auto&& fn1,auto&& fn2) {
	//evaluation order is undefined ;-;, so i can't inline this
	auto ret = std::invoke(std::forward<decltype(fn1)>(fn1), a);
	auto other = std::invoke(std::forward<decltype(fn2)>(fn2), ret);
	return std::make_pair(std::move(ret), std::move(other));
};

template<typename T>
std::pair<snowflake, T> get_then_return_id(const nlohmann::json& json) {
	auto ret = json.get<T>();
	const snowflake id = ret.id();
	return {id, std::move(ret)};
}

static inline const auto id_comp = [](auto&& a, snowflake b) { return a.id() < b; };

static inline constexpr auto get_id = [](auto&& a) { return a.id(); };


template<typename Char>
struct fmt::formatter<snowflake,Char>: fmt::formatter<uint64_t, Char> {
	
	template <typename FormatContext>
	auto format(const snowflake& val, FormatContext& ctx) {
		return formatter<uint64_t>::format(val.val, ctx);
	}
};

