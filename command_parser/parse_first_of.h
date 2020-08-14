#pragma once
#include <string_view>
#include <variant>
#include "parse_result.h"
#include <algorithm>
#include <memory>
#include <stdexcept>

template<typename T, typename... types>
static constexpr int number_of_times_type_appears_in_thing() {
	return ((int)std::is_same_v<T, types> +...);
}

//similar to variant, but can have the same type multiple times ;-;
template<typename ...results>
struct parse_first_of_result {
	static constexpr std::array<size_t, sizeof...(results)> sizes = { sizeof(results)... };
	static constexpr size_t size = *std::max_element(sizes.begin(), sizes.end());
	parse_first_of_result() = default;

	template<typename T, int n>
	parse_first_of_result(T&& val, std::integral_constant<int, n>) :
		m_idx(n) {
		static_assert(n < sizeof...(results));

		using result = std::tuple_element_t<n, std::tuple<results...>>;
		new(&m_storage) result(std::forward<T>(val));
	}

	parse_first_of_result(const parse_first_of_result& other) {
		m_idx = other.m_idx;
		if (other.has_value()) {
			other.visit([&](const auto& v) {
				using type = std::decay_t<decltype(v)>;
				new(&m_storage) type(v);
			});
		}
	}

	parse_first_of_result(parse_first_of_result&& other) noexcept {
		m_idx = other.m_idx;
		if (other.has_value()) {
			other.visit([&](auto& v) {
				using type = std::decay_t<decltype(v)>;
				new(&m_storage) type(std::move(v));
			});
		}
	}

	parse_first_of_result& operator=(const parse_first_of_result& other) {
		if (this == &other)
			return *this;
		std::destroy_at(this);
		m_idx = other.m_idx;
		if (other.has_value()) {
			other.visit([&](const auto& v) {
				using type = std::decay_t<decltype(v)>;
				new(&m_storage) type(v);
			});
		}
		return *this;
	}

	parse_first_of_result& operator=(parse_first_of_result&& other) noexcept {
		if (this == &other)
			return *this;
		std::destroy_at(this);
		m_idx = other.m_idx;
		if (other.has_value()) {
			other.visit([&](auto& v) {
				using type = std::decay_t<decltype(v)>;
				new(&m_storage) type(std::move(v));
			});
		}
		return *this;
	}

	~parse_first_of_result()noexcept {
		if (!has_value()) {
			return;
		}
		visit([](auto& a) {
			std::destroy_at(&a);
		});
	}

	[[nodiscard]] constexpr int index() const noexcept {
		return m_idx;
	}

	template<typename T>
	T& get() noexcept {
		return *(T*)std::launder(&m_storage);
	}

	template<typename T>
	const T& get() const noexcept {
		return *(T*)std::launder(&m_storage);
	}

	template<int idx>
	auto& get() noexcept {
		using type = std::tuple_element_t<idx, std::tuple<results...>>;
		return *(type*)std::launder(&m_storage);
	}

	template<int idx>
	const auto& get() const noexcept {
		using type = std::add_const_t<std::tuple_element_t<idx, std::tuple<results...>>>;
		return *(type*)std::launder(&m_storage);
	}

	template<typename fn>
	decltype(auto) visit(fn&& f) {//[[assert:m_idx!=-1]]
		using return_type = std::invoke_result_t<fn, decltype(get<0>())>;
		return visit_impl<fn, 0, return_type>(std::forward<fn>(f));
	}

	template<typename fn>
	decltype(auto) visit(fn&& f) const {//[[assert:m_idx!=-1]]	
		using return_type = std::invoke_result_t<fn, decltype(get<0>())>;
		return visit_impl<fn, 0, return_type>(std::forward<fn>(f));
	}

	[[nodiscard]] bool has_value() const noexcept {
		return m_idx != -1;
	}

	template<typename T>
	constexpr bool holds_type() const noexcept {
		static constexpr auto idx_that_have_type_T = holds_type_impl<T>();
		for (auto& a : idx_that_have_type_T) {
			if (a == m_idx) {
				return true;
			}
		}
		return false;
	}

private:
	template<typename fn, int idx, typename return_type>
	auto visit_impl(fn&& f)->return_type {
		if constexpr (idx >= sizeof...(results)) {
			/*
			if constexpr (!std::is_void_v<return_type>) {
				return return_type{};
			}*/
			throw std::runtime_error("wat");
		}
		else {
			if (idx == m_idx) {
				using type = std::tuple_element_t<idx, std::tuple<results...>>;
				return std::invoke(f, get<type>());
			}
			else {
				return visit_impl<fn, idx + 1, return_type>(std::forward<fn>(f));
			}
		}
	}

	template<typename fn, int idx, typename return_type>
	auto visit_impl(fn&& f) const->return_type {
		if constexpr (idx >= sizeof...(results)) {
			std::terminate();
		}
		else {
			if (idx == m_idx) {
				using type = std::tuple_element_t<idx, std::tuple<results...>>;
				return std::invoke(f, get<type>());
			}
			else {
				return visit_impl<fn, idx + 1, return_type>(std::forward<fn>(f));
			}
		}
	}
	template<typename T>
	static constexpr auto holds_type_impl() {
		int idx = 0;
		int idx2 = 0;
		std::array<int, number_of_times_type_appears_in_thing<T, results...>()> ret_val = {};
		((std::is_same_v<T, results> ? ret_val[idx++] = idx2++ : idx2++), ...);
		return ret_val;
	}

	alignas(16) std::array<std::byte, size> m_storage;
	int8_t m_idx = -1;
};

template<typename...T>
struct std::variant_size<parse_first_of_result<T...>> :std::integral_constant<int, sizeof...(T)> {};

static_assert(std::variant_size_v<parse_first_of_result<int, char>> == 2);

namespace parsing_stuff_details {

	template<typename T>
	struct type {
		using value_type = T;
	};

	template<typename First_parser, typename... Parsers, typename result_type, int idx>
	parse_result<result_type> parse_first_of_impl(
		std::string_view s, type<result_type> a, std::integral_constant<int, idx> b, First_parser&& first_parser, Parsers&& ... parsers) {

		auto result = std::invoke(first_parser, s);
		if (!result) {
			if constexpr (sizeof...(parsers) == 0) {
				return parse_fail();
			}
			else {
				return parse_first_of_impl(s, a, std::integral_constant<int, idx + 1>{}, std::forward<Parsers>(parsers)...);
			}
		}
		else {
			return parse_result(result_type(std::forward<parse_result_value_t<First_parser>>(result.value()), b), result.rest());
		}
	}
}

template<typename... Parsers>
auto parse_first_of(std::string_view s, Parsers&& ... parsers) {
	using namespace parsing_stuff_details;
	static_assert(sizeof...(Parsers) >= 1);
	using result_type = parse_first_of_result<parse_result_value_t<Parsers>...>;
	return parse_first_of_impl(s, type<result_type>{}, std::integral_constant<int, 0>{}, std::forward<Parsers>(parsers)...);
}

template<typename ...Parsers>
struct first_of_parser {
	explicit first_of_parser(Parsers ... p) :
		m_parsers(std::forward<Parsers>(p)...) {}

	auto operator()(std::string_view s) {
		auto bindy = [&](auto&&... parsers) {
			return parse_first_of(s, parsers...);
		};
		return std::apply(bindy, m_parsers);
	}

private:
	std::tuple<Parsers...> m_parsers;
};

template<typename...Parsers>
first_of_parser(Parsers&&...)->first_of_parser<Parsers...>;

