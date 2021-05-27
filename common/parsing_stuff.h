#pragma once
#include <string>
#include <string_view>
#include <utility>
#include <algorithm>
#include <optional>
#include <array>
#include <type_traits>
#include <cctype>
#include <variant>

template<typename range1, typename range2>
constexpr bool ci_equal(range1&& r1, range2&& r2) {
	const auto [a, b] = std::mismatch(r1.begin(), r1.end(), r2.begin(), [](const auto& a, const auto& b) {
		return std::toupper(a) == std::toupper(b);
		});
	return a == r1.end() && b == r2.end();
}

template<typename T>
struct parse_result;

struct parse_fail_with_reason {
	parse_fail_with_reason() = default;
	parse_fail_with_reason(std::string r) :
		reason(std::move(r)) {}

	std::string reason = "";
};

template<typename T>
struct propagate_parse_fail {
	parse_result<T>* failed_parse_result = nullptr;
};

template<typename T>
struct propagate_parse_fail_const {
	const parse_result<T>* failed_parse_result = nullptr;
};

template<typename T>
struct propagate_parse_fail_r_value {
	parse_result<T> failed_parse_result;
};

struct parse_fail_no_reason {

};

[[nodiscard]]
constexpr parse_fail_no_reason parse_fail() noexcept {
	return {};
}

[[nodiscard]]
inline parse_fail_with_reason parse_fail(std::string reason) {
	return parse_fail_with_reason(std::move(reason));
}

template<typename T>
struct parse_result {
	using parse_value = T;

	constexpr parse_result() = default;
	constexpr explicit parse_result(T a, std::string_view s) :
		m_self(std::in_place_index<0>, std::make_pair(std::move(a), s)) {}

	/*
	template<typename U,std::enable_if_t<std::is_convertible_v<U,T>,int> = 0>
	constexpr parse_result(parse_result<U> other) {
		if (other) {
			//*this = parse_result<U>(U(std::move(value())), rest());
			m_self = std::make_pair(U(std::move(other.value())),other.rest());
		}else if (std::holds_alternative<std::string>(other.m_self)) {
			*this =  parse_fail(std::get<std::string>(other.m_self));
		}else if (std::holds_alternative<parse_fail_no_reason>(other.m_self)) {
			*this =  parse_fail();
		}
	}
	//*/
	//constexpr parse_result(std::nullopt_t) {}

	constexpr parse_result(parse_fail_no_reason a) :m_self(a) {}

	parse_result(parse_fail_with_reason r) :m_self(std::move(r.reason)) {}

	template<typename U>
	parse_result(propagate_parse_fail<U> f) {
		if (f.failed_parse_result->m_self.index() == 1) {
			m_self = std::move(std::get<1>(f.failed_parse_result->m_self));
		}
		else {
			m_self = parse_fail_no_reason();
		}
	}

	template<typename U>
	parse_result(propagate_parse_fail_const<U> f) {
		if (f.failed_parse_result->m_self.index() == 1) {
			m_self = std::get<1>(f.failed_parse_result->m_self);
		}
		else {
			m_self = parse_fail_no_reason();
		}
	}

	template<typename U>
	parse_result(propagate_parse_fail_r_value<U> f) {
		if (f.failed_parse_result->m_self.index() == 1) {
			m_self = std::move(std::get<1>(f.failed_parse_result->m_self));
		}
		else {
			m_self = parse_fail_no_reason();
		}
	}

	constexpr explicit operator bool()const noexcept {
		return success();
	}
	//TODO: remove these 4 functions eventually
	constexpr std::pair<T, std::string_view>* operator->() {
		return &(**this);
	}

	constexpr const std::pair<T, std::string_view>* operator->()const noexcept {
		return &(**this);
	}

	constexpr auto& operator*() noexcept {
		return std::get<0>(m_self);
	}

	constexpr const auto& operator*() const noexcept {
		return std::get<0>(m_self);
	}

	constexpr bool success()const noexcept {
		return std::holds_alternative<std::pair<T, std::string_view>>(m_self);
	}

	constexpr T& value() noexcept {
		return std::get<0>(m_self).first;
	}

	constexpr const T& value() const noexcept {
		return std::get<0>(m_self).first;
	}

	constexpr std::string_view rest()const noexcept {
		return std::get<0>(m_self).second;
	}

	constexpr std::string_view fail_reason() const {
		if (m_self.index() == 1) {
			return std::get<std::string>(m_self);
		}
		else {
			return "";
		}
	}

	//*
	template<typename U>requires std::is_convertible_v<T, U>
	operator parse_result<U>() const & {
		if (bool(*this)) {
			return parse_result<U>(U(value()), rest());
		}
		else if (std::holds_alternative<std::string>(m_self)) {
			return parse_fail(std::get<std::string>(m_self));
		}
		else if (std::holds_alternative<parse_fail_no_reason>(m_self)) {
			return parse_fail();
		}
		else {
			return parse_result<U>();
		}
	}

	template<typename U>requires std::is_convertible_v<T, U>
	operator parse_result<U>() & {
		if (bool(*this)) {
			return parse_result<U>(U(value()), rest());
		}
		else if (std::holds_alternative<std::string>(m_self)) {
			return parse_fail(std::get<std::string>(m_self));
		}
		else if (std::holds_alternative<parse_fail_no_reason>(m_self)) {
			return parse_fail();
		}
		else {
			return parse_result<U>();
		}
	}

	template<typename U> requires std::is_convertible_v<T, U>
	operator parse_result<U>() && {
		if (bool(*this)) {
			return parse_result<U>(U(std::move(value())), rest());
		}
		else if (std::holds_alternative<std::string>(m_self)) {
			return parse_fail(std::get<std::string>(m_self));
		}
		else if (std::holds_alternative<parse_fail_no_reason>(m_self)) {
			return parse_fail();
		}
		else {
			return parse_result<U>();
		}
	}

	template<typename U>requires std::is_convertible_v<T, U>
	operator parse_result<U>()const && {
		if (bool(*this)) {
			return parse_result<U>(U(value()), rest());
		}
		else if (std::holds_alternative<std::string>(m_self)) {
			return parse_fail(std::get<std::string>(m_self));
		}
		else if (std::holds_alternative<parse_fail_no_reason>(m_self)) {
			return parse_fail();
		}
		else {
			return parse_result<U>();
		}
	}

	//*/
private:
	/* parse_fail_no_reason is so it'll work in constexpr, std::string isn't constexpr*/
	std::variant<std::pair<T, std::string_view>, std::string, parse_fail_no_reason, std::monostate> m_self = std::monostate{};
	template<typename>
	friend struct parse_result;
};

template<>
struct parse_result <void> {
	using parse_value = void;

	constexpr parse_result() = default;
	constexpr explicit parse_result(std::string_view s) :m_self(std::in_place_index<0>, s) {}

	constexpr parse_result(std::nullopt_t) {}

	constexpr parse_result(parse_fail_no_reason a) : m_self(a) {}

	parse_result(parse_fail_with_reason r) :m_self(std::in_place_index<1>, std::move(r.reason)) {}

	constexpr explicit operator bool()const noexcept {
		return success();
	}

	constexpr const auto& operator*() const {
		return std::get<0>(m_self);
	}

	constexpr const std::string_view* operator->() const {
		return &(**this);
	}

	constexpr bool success()const noexcept {
		return std::holds_alternative<std::string_view>(m_self);
	}

	constexpr std::string_view rest()const {
		return std::get<0>(m_self);
	}

	std::string_view fail_reason()const noexcept {
		if (m_self.index() == 1) {
			return std::get<1>(m_self);
		}
		else {
			return "";
		}
	}

private:
	std::variant<std::string_view, std::string, parse_fail_no_reason, std::monostate> m_self = std::monostate{};
	template<typename>
	friend struct parse_result;
};

template<typename T>
[[nodiscard]]
inline propagate_parse_fail<T> parse_fail(parse_result<T>& p) {
	return propagate_parse_fail<T>{&p};
}

template<typename T>
[[nodiscard]]
inline propagate_parse_fail_const<T> parse_fail(const parse_result<T>& p) {
	return propagate_parse_fail_const<T>{&p};
}

template<typename T>
[[nodiscard]]
inline propagate_parse_fail_r_value<T> parse_fail(parse_result<T>&& p) {
	return propagate_parse_fail<T>{std::move(p)};
}

//keeps trying to parse until it successfully parses, all parsers must return the same result
template<typename first_parser, typename...parsers_t>
[[nodiscard]]
constexpr auto try_parse_multiple(const std::string_view s, first_parser&& first, parsers_t&& ...rest) {
	const auto result = first(s);
	if (result)
		return result;
	return try_parse_multiple(s, std::forward<parsers_t>(rest)...);
}

template<typename first_parser>
[[nodiscard]]
constexpr auto try_parse_multiple(const std::string_view s, first_parser&& first) {
	return first(s);
}

struct try_parse_multiple_t {
	template<typename...parsers_t>
	constexpr decltype(auto) operator()(const std::string_view s, parsers_t&& ...rest)const {
		return try_parse_multiple(s, std::forward<parsers_t>(rest)...);
	}
};

static constexpr try_parse_multiple_t try_parse_multiple_a;
[[nodiscard]]
static inline constexpr bool is_whitespace_char(char c) noexcept {
	return c == ' ' || c == '\n' || c == '\t' || c == '\v' || c == '\f' || c == '\r';
}
[[nodiscard]]
static constexpr bool is_alpha_char(char c) noexcept {
	return c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z';
}
[[nodiscard]]
static constexpr bool is_numeric_char(char c)noexcept {
	return c >= '0' && c <= '9';
}

template<typename T>
struct parse_result_of {
	using value_type = std::decay_t<decltype(std::declval<T>().operator()(std::declval<std::string_view>()).value())>;
};

template<typename T>
using parse_result_of_t = std::decay_t<decltype(std::declval<T>().operator()(std::declval<std::string_view>()).value()) >;

/*
parses using n parsers, each after each other, returns results in tuple
*/
template<typename first_parser, typename ...rest_parsers>
constexpr auto parse_multi_consecutive(const std::string_view s, first_parser&& f, rest_parsers&& ... rest)
->parse_result<std::tuple<parse_result_of_t<first_parser>, parse_result_of_t<rest_parsers>...>>
{
	auto t = f(s);
	if (!t)
		return parse_fail();
	auto y = parse_multi_consecutive(t.rest(), std::forward<rest_parsers>(rest)...);
	if (!y)
		return parse_fail();
	return parse_result(std::tuple_cat(std::make_tuple(std::move(t->first)), std::move(y->first)), y.rest());

}

template<typename first_parser>
constexpr auto parse_multi_consecutive(const std::string_view s, first_parser&& f)
->parse_result<std::tuple<parse_result_of_t<first_parser>>>
{
	auto t = f(s);
	if (!t)
		return parse_fail();
	return parse_result(std::make_tuple(std::move(t.value())), t.rest());
}

//parses using 2 parsers, one after each other, then joins them
template<typename parser1, typename parser2, typename join_fn>
constexpr auto parse_consecutive_and_join(const std::string_view s, parser1&& p1, parser2&& p2, join_fn&& fn)
->parse_result<std::invoke_result_t<join_fn, parse_result_of_t<parser1>, parse_result_of_t<parser2>>>
{
	auto t = p1(s);
	if (!t)
		return parse_fail();
	auto o = p2(t.rest());
	if (!o)
		return parse_fail();
	return parse_result(std::invoke(fn, std::forward<decltype(t.value())>(t.value()), std::forward<decltype(o.value())>(o.value())), o.rest());
}

template<typename ...T>
struct multi_parser {
	constexpr multi_parser() = default;

	constexpr multi_parser(T... stuff) :m_parsers(std::move(stuff)...) {}
	constexpr decltype(auto) operator()(std::string_view s) {
		return std::apply([&](auto&& ...a) {return try_parse_multiple(s, std::forward<decltype(a)>(a)...); }, m_parsers);
	}
private:
	std::tuple<T...> m_parsers;
};

template<typename P1, typename P2, typename fn>
struct consecutive_parser {
	constexpr consecutive_parser() = default;
	constexpr consecutive_parser(P1 a, P2 b, fn c) :
		m_parser1(std::move(a)),
		m_parser2(std::move(b)),
		m_join_fn(std::move(c)) {}

	constexpr decltype(auto) operator()(std::string_view s) {
		return parse_consecutive_and_join(s, m_parser1, m_parser2, m_join_fn);
	}
private:
	P1 m_parser1 = {};
	P2 m_parser2 = {};
	fn m_join_fn = {};
};

template<typename P1, typename P2, typename fn>
consecutive_parser(P1&&, P2&&, fn&&)->consecutive_parser<P1, P2, fn>;

template<size_t i, size_t max = std::numeric_limits<size_t>::max()>
struct return_nth {
	template<typename...Args>requires (sizeof...(Args) <= max && (sizeof...(Args) > i))
	constexpr auto operator()(Args && ... stuff)noexcept(noexcept(std::get<i>(std::forward_as_tuple(std::forward<Args>(stuff)...))))
		->std::tuple_element_t<i, decltype(std::forward_as_tuple(std::forward<Args>(stuff)...))>
	{
		return std::get<i>(std::forward_as_tuple(std::forward<Args>(stuff)...));
	}
};

using ignore_left = return_nth<1, 2>;

using ignore_right = return_nth<0, 2>;

//parses whitespace
struct whitespace_parser_t {
	parse_result<int> operator()(std::string_view s) const noexcept {
		auto num_spaces = (int)std::distance(s.begin(), std::find_if_not(s.begin(), s.end(), is_whitespace_char));
		return parse_result(num_spaces, s.substr(num_spaces));
	}
};

static constexpr inline whitespace_parser_t whitespace_parser;

//parses a single char
template<bool fail_on_fail = true>
struct char_parser {
	constexpr char_parser(char a) :c(a) {}
	constexpr parse_result<bool> operator()(std::string_view s) const noexcept {
		if (s.empty())	return parse_fail();

		const bool r = s.front() == c;

		if constexpr (fail_on_fail)
			if (!r)
				return parse_fail();

		s.remove_prefix(r);//1 if it's there, 0 otherwise
		return parse_result(r, s);
	}

	char c;
};

/*
parses "abcde" in "abcde qwe asd"
"qwe" in "qwe2"
*/
struct alpha_word_parser {
	parse_result<std::string_view> operator()(std::string_view s) const noexcept {
		const auto it = std::find_if_not(s.begin(), s.end(), is_alpha_char);
		const size_t size = std::distance(s.begin(), it);
		return parse_result(s.substr(0, size), s.substr(size));
	}
};

struct alpha_numeric_word_parser {
	parse_result<std::string_view> operator()(std::string_view s) const noexcept {
		const auto it = std::find_if_not(s.begin(), s.end(), [](char c) {
			return is_alpha_char(c) || is_numeric_char(c);
			});
		const size_t size = std::distance(s.begin(), it);
		return parse_result(s.substr(0, size), s.substr(size));
	}
};

//similar to variant, but can have the same type multiple times ;-;
template<typename ...results>
struct parse_first_of_result {
	static constexpr std::array<size_t, sizeof...(results)> sizes = { sizeof(results)... };
	static constexpr size_t size = *std::max_element(sizes.begin(), sizes.end());
	parse_first_of_result() = default;

	template<typename T, int n>
	parse_first_of_result(T&& val, std::integral_constant<int, n>) :m_idx(n) {
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

	~parse_first_of_result() {
		if (!has_value()) {
			return;
		}
		visit([](auto& a) {
			std::destroy_at(&a);
			});
	}

	constexpr int index()const noexcept {
		return m_idx;
	}

	template<typename T>
	T& get() noexcept {
		return *(T*)std::launder(&m_storage);
	}

	template<typename T>
	const T& get()const noexcept {
		return *(T*)std::launder(&m_storage);
	}

	template<int idx>
	auto& get() noexcept {
		using type = std::tuple_element_t<idx, std::tuple<results...>>;
		return *(type*)std::launder(&m_storage);
	}

	template<int idx>
	const auto& get()const noexcept {
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

	bool has_value()const noexcept {
		return m_idx != -1;
	}

private:
	template<typename fn, int idx, typename return_type>
	decltype(auto) visit_impl(fn&& f) {
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
	decltype(auto) visit_impl(fn&& f)const {
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

	std::array<std::byte, size> m_storage;
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
		std::string_view s, type<result_type> a, std::integral_constant<int, idx> b, First_parser&& first_parser, Parsers&& ... parsers)
	{
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
			return parse_result(result_type(std::forward<parse_result_of_t<First_parser>>(result.value()), b), result.rest());
		}
	}
}

template<typename... Parsers>
auto parse_first_of(std::string_view s, Parsers&& ... parsers) {
	using namespace parsing_stuff_details;
	static_assert(sizeof...(Parsers) >= 1);
	using result_type = parse_first_of_result<parse_result_of_t<Parsers>...>;
	return parse_first_of_impl(s, type<result_type>{}, std::integral_constant<int, 0>{}, std::forward<Parsers>(parsers)...);
}

struct parse_upto_n {
	constexpr parse_upto_n(size_t a) :n(a) {}

	parse_result<std::string_view> operator()(std::string_view s)const {
		if (s.size() >= n)
			return parse_result(s.substr(0, n), s.substr(n));
		else
			return parse_result(s, s.substr(s.size()));
	}

	size_t n;
};

template<bool fail_on_fail = true>
struct str_parser {

	constexpr str_parser(std::string_view s) :str(s) {}

	constexpr parse_result<bool> operator()(std::string_view data) const {
		if (data.size() < str.size() ||
			data.substr(0, str.size()) != str)
		{
			if constexpr (fail_on_fail)
				return parse_fail();
			else
				return parse_result(false, data);
		}
		return parse_result(true, data.substr(str.size()));
	}
	std::string_view str;
};

template<bool fail_on_fail = true>
struct ci_str_parser {
	constexpr ci_str_parser(std::string_view s) :str(s) {}

	constexpr parse_result<bool> operator()(std::string_view data) {
		if (data.size() < str.size() ||
			!ci_equal(data.substr(0, str.size()), str))
		{
			if constexpr (fail_on_fail)
				return parse_fail();
			else
				return parse_result(false, data);
		}
		return parse_result(true, data.substr(str.size()));

	}

	std::string_view str;
};

template<typename parser_t>
struct optional_parser {
	constexpr optional_parser() = default;
	constexpr optional_parser(parser_t a) :parser(std::move(a)) {}

	constexpr parse_result<std::optional<parse_result_of_t<parser_t>>> operator()(std::string_view s) {
		auto t = parser(s);
		if (t) {
			return parse_result(std::optional(std::move(t.value())), t.rest());
		}
		else {
			return parse_result<std::optional<parse_result_of_t<parser_t>>>(std::nullopt, s);
		}
	}
	parser_t parser;
};

struct quote_parser {
	parse_result<char> operator()(std::string_view s)const {
		if (!s.empty() &&
			(s.front() == '"' || s.front() == '\''))
		{
			const char r = s.front();
			s.remove_prefix(1);
			return parse_result(r, s);
		}
		return parse_fail();
	}
};

/*
parses things until pred is false
*/
template<typename fn>
struct predicate_parser {
	constexpr predicate_parser() = default;
	constexpr predicate_parser(fn a) :pred(std::move(a)) {}

	constexpr parse_result<std::string_view> operator()(std::string_view s) {
		auto it = std::find_if_not(s.begin(), s.end(), pred);
		const size_t size = std::distance(s.begin(), it);
		return parse_result(s.substr(0, size), s.substr(size));
	}

	fn pred;
};

template<typename fn>
predicate_parser(fn&&)->predicate_parser<fn>;

template<typename fn>
struct predicate_at_least_1_parser {//rename later ;-;
	constexpr predicate_at_least_1_parser() = default;
	constexpr predicate_at_least_1_parser(fn a) :f(std::move(a)) {}

	constexpr parse_result<std::string_view> operator()(std::string_view s) {
		auto it = std::find_if_not(s.begin(), s.end(), f);
		const size_t size = std::distance(s.begin(), it);
		if (!size)
			return parse_fail();
		return parse_result(s.substr(0, size), s.substr(size));
	}

	fn f;
};

template<typename fn>
predicate_at_least_1_parser(fn&&)->predicate_at_least_1_parser<fn>;

template<typename fn>
struct parse_until {
	parse_until(fn f_t) :f(std::move(f_t)) {}

	parse_result<std::string_view> operator()(std::string_view s) const {
		const auto it = std::find_if(s.begin(), s.end(), f);
		const auto count = std::distance(s.begin(), it);
		return parse_result(s.substr(0, count), s.substr(count));
	}

	fn f;
};

template<typename fn>
parse_until(fn&&)->parse_until<fn>;

struct quote_string_parser {
	parse_result<std::string_view> operator()(std::string_view s) {
		if (s.empty() || (s.front() != '"' && s.front() != '\''))
			return parse_fail();

		const char expected_quote = s.front();
		const auto last_char_it = std::invoke([&]() {
			for (int i = 1; i < (int)s.size(); ++i) {
				if (s[i] == expected_quote) {
					return s.begin() + (i - 1);
				}
				if (s[i] == '\\') {
					//skip next char
					++i;//can't use algorithms or range for loop cuz of this
				}
			}
			return s.end();
			});

		if (last_char_it == s.end())
			return parse_fail();
		const size_t size = std::distance(s.begin(), last_char_it);

		//+1 skipes the first "/', +2 skips both
		return parse_result(s.substr(1, size), s.substr(size + 2));
	}
};