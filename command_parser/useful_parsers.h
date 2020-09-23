#pragma once
#include "parse_result.h"
//TODO rename this file

[[nodiscard]]
static inline constexpr bool is_whitespace_char(const char c) noexcept {
	return c == ' ' || c == '\n' || c == '\t' || c == '\v' || c == '\f' || c == '\r';
}

[[nodiscard]]
static constexpr bool is_alpha_char(const char c) noexcept {
	return c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z';
}

[[nodiscard]]
static constexpr bool is_numeric_char(const char c) noexcept {
	return c >= '0' && c <= '9';
}

template<typename range1, typename range2>
constexpr bool ci_equal(range1&& r1, range2&& r2) {
	//if constexpr(is_sized_range<range1>)
	if (r1.size() != r2.size()) {
		return false;
	}
	const auto [a, b] = std::mismatch(r1.begin(), r1.end(), r2.begin(), [](const auto& a, const auto& b) {
		return std::toupper(a) == std::toupper(b);
	});
	return a == r1.end() && b == r2.end();
}

template<size_t i, size_t max = std::numeric_limits<size_t>::max()>
struct return_nth {
	template<typename...Args, std::enable_if_t<sizeof...(Args) <= max && (sizeof...(Args) > i), int>  = 0>
	constexpr auto operator()(Args&& ... stuff) noexcept(noexcept(std::get<i>(std::forward_as_tuple(std::forward<Args>(stuff)...))))
	->std::tuple_element_t<i, decltype(std::forward_as_tuple(std::forward<Args>(stuff)...))> {

		return std::get<i>(std::forward_as_tuple(std::forward<Args>(stuff)...));
	}

	template<typename...Args, std::enable_if_t<sizeof...(Args) <= max && (sizeof...(Args) > i), int>  = 0>
	constexpr auto operator()(std::tuple<Args...>& stuff) noexcept->std::tuple_element_t<i, std::tuple<Args...>>& {
		return std::get<i>(stuff);
	}

	template<typename...Args, std::enable_if_t<sizeof...(Args) <= max && (sizeof...(Args) > i), int>  = 0>
	constexpr auto operator()(std::tuple<Args...>&& stuff) noexcept->std::tuple_element_t<i, std::tuple<Args...>> {
		return std::move(std::get<i>(stuff));
	}
};

using ignore_left = return_nth<1, 2>;

using ignore_right = return_nth<0, 2>;

template<size_t i, size_t... is>
constexpr auto append_idx_to_front(std::index_sequence<is...>) {
	return std::index_sequence<i, is...>();
};

static_assert(
	std::is_same_v<decltype(append_idx_to_front<0>(std::make_index_sequence<2>())),
				   std::index_sequence<0, 0, 1>
	>
);

template<int... idxs>
struct remove_idxs_context {
	static constexpr std::array<size_t, sizeof...(idxs)> idxs_ignoreing = {idxs...};

	static constexpr bool is_idx_to_ignore_fn(size_t i) {
		//std::contains, but constexpr
		for (auto idx : idxs_ignoreing) {
			if (i == idx) {
				return true;
			}
		}
		return false;
	}

	static constexpr auto remove_idxs(std::index_sequence<> a) {
		return a;
	}


	template<size_t idx1, size_t ... rest>
	static constexpr auto remove_idxs(std::index_sequence<idx1, rest...>) {
		if constexpr (is_idx_to_ignore_fn(idx1)) {
			return remove_idxs(std::index_sequence<rest...>());
		} else {
			return append_idx_to_front<idx1>(remove_idxs(std::index_sequence<rest...>()));
		}
	}


};

template<size_t ...idxs_removing, size_t ... idxs>
constexpr auto remove_idxs2(std::index_sequence<idxs_removing...>, std::index_sequence<idxs...> a) {
	return remove_idxs_context<idxs_removing...>::remove_idxs(a);
}

static_assert(
	std::is_same_v<
		decltype(remove_idxs2(std::make_index_sequence<5>(), std::make_index_sequence<7>())),
		std::index_sequence<5, 6>
	>
);

static_assert(
	std::is_same_v<
		decltype(remove_idxs2(std::index_sequence<0, 2>(), std::make_index_sequence<3>())),
		std::index_sequence<1>
	>
);

static_assert(
	std::is_same_v<
		decltype(remove_idxs2(std::index_sequence<0, 2>(), std::make_index_sequence<3>())),
		std::index_sequence<1>
	>
);

template<int... idxs>
struct ignore_tuple_idxs {
	static constexpr std::array<int, sizeof...(idxs)> idxs_ignoreing = {idxs...};

	static constexpr bool is_idx_to_ignore_fn(int i) {
		//std::contains, but constexpr
		for (auto idx : idxs_ignoreing) {
			if (i == idx) {
				return true;
			}
		}
		return false;
	}

	template<int i>
	static constexpr bool is_idx_to_ignore = is_idx_to_ignore_fn(i);

	template<typename... Ts>
	constexpr auto operator()(std::tuple<Ts...>&& tuple) {
		constexpr int max_idx = sizeof...(Ts);
		return do_stuff<max_idx>(std::move(tuple));
	}

	template<typename... Ts>
	constexpr auto operator()(const std::tuple<Ts...>&& tuple) {
		constexpr int max_idx = sizeof...(Ts);
		return do_stuff<max_idx>(tuple);
	}

	template<typename... Ts>
	constexpr auto operator()(std::tuple<Ts...>& tuple) {
		constexpr int max_idx = sizeof...(Ts);
		return do_stuff<max_idx>(tuple);
	}

	template<typename... Ts>
	constexpr auto operator()(const std::tuple<Ts...>& tuple) {
		constexpr int max_idx = sizeof...(Ts);
		return do_stuff<max_idx>(tuple);
	}

	/*
	template<int i, typename tuple1_t, typename tuple2_t>
	constexpr auto do_stuff(tuple1_t&& original_tuple, tuple2_t t) {
		if constexpr (i == -1) {
			return t;
		}
		else if constexpr (is_idx_to_ignore<i>) {
			return do_stuff<i - 1>(std::forward<tuple1_t>(original_tuple), std::move(t));
		}
		else {
			auto val = std::get<i>(original_tuple);
			return do_stuff<i - 1>(std::forward<tuple1_t>(original_tuple), std::tuple_cat(std::make_tuple(std::move(val)), std::move(t)));
		}
	}
	*/

	template<size_t i, typename tuple1_t>
	constexpr auto do_stuff(tuple1_t&& original_tuple) {
		constexpr auto idx_keeping = remove_idxs2(std::index_sequence<idxs...>(), std::make_index_sequence<i>());
		static_assert(idx_keeping.size() + idxs_ignoreing.size() == i);
		return do_stuff_impl(std::forward<tuple1_t>(original_tuple), idx_keeping);
	}

	template<typename tuple, size_t... idxs_to_keep>
	constexpr auto do_stuff_impl(tuple&& t, std::index_sequence<idxs_to_keep...>) {
		return std::make_tuple(std::get<idxs_to_keep>(std::forward<tuple>(t))...);
	}
};

//parses whitespace
struct whitespace_parser_t {
	parse_result<int> operator()(std::string_view s) const noexcept {
		const auto num_spaces = (int)std::distance(s.begin(), std::find_if_not(s.begin(), s.end(), &is_whitespace_char));
		return parse_result(num_spaces, s.substr(num_spaces));
	}
};

static constexpr inline whitespace_parser_t whitespace_parser;

struct non_optional_whitespace_parser {

	parse_result<int> operator()(std::string_view s) const {
		if (s.empty() || !is_whitespace_char(s.front())) {
			return parse_fail();
		}
		return whitespace_parser(s);
	}
};

struct int_parser {
	parse_result<int> operator()(std::string_view s) const {
		int r = 0;
		const auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), r);
		if (ptr == s.data()) {
			return parse_fail();
		} else {
			const ptrdiff_t used_chars = ptr - s.data();
			s.remove_prefix(used_chars);
			return parse_result(r, s);
		}
	}
};

template<typename T> //requires std::is_numeric<T>
struct integral_parser {
	parse_result<T> operator()(std::string_view s) const {
		T r = 0;
		const auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), r);
		if (ptr == s.data()) {
			return parse_fail();
		}
		else {
			const ptrdiff_t used_chars = ptr - s.data();
			s.remove_prefix(used_chars);
			return parse_result(r, s);
		}
	}
};

struct uint64_parser {
	parse_result<uint64_t> operator()(std::string_view s) const {
		uint64_t r = 0;
		const auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), r);
		if (ptr == s.data()) {
			return parse_fail();
		}
		else {
			const ptrdiff_t used_chars = ptr - s.data();
			s.remove_prefix(used_chars);
			return parse_result(r, s);
		}
	}
};

struct double_parser {
	parse_result<double> operator()(std::string_view s) const {
		double r = 0;
		const auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), r);
		if (ptr == s.data()) {
			return parse_fail();
		} else {
			const ptrdiff_t used_chars = ptr - s.data();
			s.remove_prefix(used_chars);
			return parse_result(r, s);
		}
	}
};


struct int_or_double_parser {
	parse_result<std::variant<int, double>> operator()(std::string_view s) const {
		if (s.empty()) {
			return parse_fail();
		}
		auto result = int_parser()(s);
		if (result) {
			auto [value, rest] = *result;

			if (!rest.empty() && (rest.front() == '.' || rest.front() == 'e' || rest.front() == 'E')) {
				return double_parser()(s);
			} else {
				return std::move(result);
			}
		} else {
			return double_parser()(s);
		}
	}
};

struct alpha1_parser {
	parse_result<char> operator()(std::string_view s) const {
		if (s.empty() || !is_alpha_char(s.front())) {
			return parse_fail();
		}
		return parse_result(s.front(), s.substr(1));
	}
};

struct alpha_numeric1_parser {
	parse_result<char> operator()(std::string_view s) const {
		if (s.empty() || !is_numeric_char(s.front()) || !is_alpha_char(s.front())) {
			return parse_fail();
		}
		return parse_result(s.front(), s.substr(1));
	}
};

template<int n>
struct chars1_parser {
	chars1_parser() = delete;

	template<typename... ch>
	explicit chars1_parser(ch ... chr) :
		chars{(char)chr...} { }

	parse_result<char> operator()(std::string_view input) {
		if (!input.empty() &&
			std::find(chars.begin(), chars.end(), input.front()) != chars.end())//replace with contains() ;-; 
		{
			return parse_result(input.front(), input.substr(1));
		} else {
			return parse_fail();
		}
	}

	std::array<char, n> chars;
};

template<>
struct chars1_parser<0> {
	//no chars
	chars1_parser() = default;

	parse_result<char> operator()(std::string_view input) const {
		return parse_fail();
	}

};

template<>
struct chars1_parser<-1> {
	chars1_parser() = delete;
	//gib aggragate thingy

	explicit chars1_parser(std::string_view c) :
		chars(c) {}

	parse_result<char> operator()(std::string_view input) const {
		if (input.empty() ||
			std::find(chars.begin(), chars.end(), input.front()) == chars.end())//replace with !contains() ;-; 
		{
			return parse_fail();
		}
		return parse_result(input.front(), input.substr(1));
	}

	std::string_view chars;
};

template<typename... ch, std::enable_if_t<std::disjunction_v<std::is_convertible<ch, char> ...>, int>  = 0>
chars1_parser(ch ...)->chars1_parser<sizeof...(ch)>;

chars1_parser(std::string_view)->chars1_parser<-1>;

chars1_parser(std::string)->chars1_parser<-1>;

chars1_parser(const char*)->chars1_parser<-1>;

chars1_parser()->chars1_parser<0>;

template<typename fn>
struct parse_until {
	explicit parse_until(fn f_t) :
		f(std::move(f_t)) {}

	parse_result<std::string_view> operator()(std::string_view s) const {
		const auto it = std::find_if(s.begin(), s.end(), f);
		const auto count = std::distance(s.begin(), it);
		return parse_result(s.substr(0, count), s.substr(count));
	}

	fn f;
};

template<typename fn>
parse_until(fn&&)->parse_until<fn>;

template<int size>
struct parse_until_char {

	template<typename... Char>
	explicit parse_until_char(Char... chars_) :
		chars{chars_...} { }

	parse_result<std::string_view> operator()(std::string_view s) {
		if (s.empty()) {
			return parse_fail();
		}
		auto it = std::find_if(s.begin(), s.end(), [&](auto c) {//gib std::contains and abbriviated lambdas ;-;
			return std::find(chars.begin(), chars.end(), c) != chars.end();
		});

		if (it == s.end()) {
			return parse_fail();
		} else {
			const size_t len = std::distance(s.begin(), it);
			return parse_result(s.substr(0, len), s.substr(len));
		}
	}

	std::array<char, size> chars = {};
};

template<typename... Char>
parse_until_char(Char ...)->parse_until_char<sizeof...(Char)>;

struct nothing_parser {
	parse_result<bool> operator()(std::string_view s) const {
		return parse_result(true, s);
	}
};

struct upto_n_parser {
	constexpr explicit upto_n_parser(size_t a) :
		n(a) {}

	parse_result<std::string_view> operator()(std::string_view s) const {
		if (s.size() >= n)
			return parse_result(s.substr(0, n), s.substr(n));
		else
			return parse_result(s, s.substr(s.size()));
	}

	size_t n;
};

template<bool fail_on_fail = true>
struct str_parser {

	constexpr explicit str_parser(std::string_view s) :
		str(s) {}

	constexpr parse_result<std::string_view> operator()(std::string_view data) const {
		if (!data.starts_with(str)) {
			if constexpr (fail_on_fail)
				return parse_fail();
			else
				return parse_result("", data);
		}
		return parse_result(str, data.substr(str.size()));
	}

	std::string_view str;
};

template<bool fail_on_fail = true>
struct ci_str_parser {
	constexpr explicit ci_str_parser(std::string_view s) :
		str(s) {}

	constexpr parse_result<bool> operator()(std::string_view data) {
		if (data.size() < str.size() ||
			!ci_equal(data.substr(0, str.size()), str)) {
			if constexpr (fail_on_fail)
				return parse_fail();
			else
				return parse_result(false, data);
		}
		return parse_result(true, data.substr(str.size()));

	}

	std::string_view str;
};


struct quote_parser {
	parse_result<char> operator()(std::string_view s) const {
		if (!s.empty() &&
			(s.front() == '"' || s.front() == '\'')) {

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

	constexpr explicit predicate_parser(fn a) :
		pred(std::move(a)) {}

	constexpr parse_result<std::string_view> operator()(std::string_view s) {
		auto it = std::find_if_not(s.begin(), s.end(), pred);
		const size_t size = std::distance(s.begin(), it);
		return parse_result(s.substr(0, size), s.substr(size));
	}

	fn pred;
};

//parses a single char
template<bool fail_on_fail = true>
struct char_parser {
	constexpr explicit char_parser(char a) :
		c(a) {}

	constexpr parse_result<bool> operator()(std::string_view s) const noexcept {
		if (s.empty())
			return parse_fail();

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
		if (size == 0) {
			return parse_fail();
		}
		return parse_result(s.substr(0, size), s.substr(size));
	}
};

struct alpha_numeric_word_parser {
	parse_result<std::string_view> operator()(std::string_view s) const noexcept {
		const auto it = std::find_if_not(s.begin(), s.end(), [](char c) {
			return is_alpha_char(c) || is_numeric_char(c);
		});
		const size_t size = std::distance(s.begin(), it);
		if (size == 0) {
			return parse_fail();
		}
		return parse_result(s.substr(0, size), s.substr(size));
	}
};

template<typename fn>
predicate_parser(fn&&)->predicate_parser<fn>;

template<typename fn>
struct predicate_at_least_1_parser {//rename later ;-;
	constexpr predicate_at_least_1_parser() = default;

	constexpr explicit predicate_at_least_1_parser(fn a) :
		f(std::move(a)) {}

	constexpr parse_result<std::string_view> operator()(std::string_view s) {
		auto it = std::find_if_not(s.begin(), s.end(), f);
		const size_t size = std::distance(s.begin(), it);
		if (size == 0)
			return parse_fail();
		return parse_result(s.substr(0, size), s.substr(size));
	}

	fn f;
};

template<typename fn>
predicate_at_least_1_parser(fn&&)->predicate_at_least_1_parser<fn>;


struct quote_string_parser {
	parse_result<std::string_view> operator()(std::string_view s) {
		if (s.empty() || (s.front() != '"' && s.front() != '\''))
			return parse_fail();

		const char expected_quote = s.front();
		const auto last_char_it = std::invoke([&]() {
			for (auto it = s.begin() + 1; it != s.end(); ++it) {
				if (*it == expected_quote) {

					return it - 1;
				}
				if (*it == '\\') {
					//skip next char
					++it;//can't use algorithms or range for loop cuz of this
				}
			}
			return s.end();
		});

		if (last_char_it == s.end())
			return parse_fail();
		const size_t size = std::distance(s.begin(), last_char_it);

		//+1 skipes the first " or ', +2 skips both
		return parse_result(s.substr(1, size), s.substr(size + 2));
	}
};

//TODO: rename?
template<int num, typename>
struct parse_first_of_multi_strings {

	template<typename ...strs>
	explicit parse_first_of_multi_strings(strs&&... a) :
		strings{{std::string_view(a)...}} { }

	parse_result<std::string_view> operator()(std::string_view s) const {
		auto it = std::find_if(strings.begin(), strings.end(), [&](auto str) {
			return s.starts_with(str);
		});
		if (it == strings.end()) {
			return parse_fail();
		}
		auto& matched_string = *it;
		const auto r = s.substr(0, matched_string.size());
		s.remove_prefix(matched_string.size());
		return parse_result(r, s);
	}

	std::array<std::string_view, num> strings;
};

template<typename C>
struct parse_first_of_multi_strings<-1, C> {

	explicit parse_first_of_multi_strings(C a) :
		strings(std::forward<C>(a)) {}

	parse_result<std::string_view> operator()(std::string_view s) const {
		auto it = std::find_if(strings.begin(), strings.end(), [&](auto str) {
			return s.starts_with(str);
		});
		if (it == strings.end()) {
			return parse_fail();
		}
		auto& matched_string = *it;
		const auto r = s.substr(0, matched_string.size());
		s.remove_prefix(matched_string.size());
		return parse_result(r, s);
	}

	C strings;
};

template<typename...strs, std::enable_if_t<(((std::is_convertible_v<strs, std::string_view>) && ...)), int>  = 0>
parse_first_of_multi_strings(strs&&...)->parse_first_of_multi_strings<sizeof...(strs), void>;

template<typename C>
parse_first_of_multi_strings(C&&)->parse_first_of_multi_strings<-1, C>;

namespace parse_literals {
	constexpr char_parser<true> operator""_p(char c) {
		return char_parser<true>(c);
	}

	constexpr str_parser<true> operator""_p(const char* c,size_t s) {
		return str_parser<true>(std::string_view(c,s));
	}
}

