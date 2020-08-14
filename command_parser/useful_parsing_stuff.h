#pragma once
#include "parse_result.h"
//TODO rename file

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
	constexpr decltype(auto) operator()(const std::string_view s, parsers_t&& ...rest) const {
		return try_parse_multiple(s, std::forward<parsers_t>(rest)...);
	}
};

static constexpr try_parse_multiple_t try_parse_multiple_a;

/*
parses using n parsers, each after each other, returns results in tuple
*/
template<typename first_parser, typename ...rest_parsers>
constexpr auto parse_consecutive(const std::string_view s, first_parser&& f, rest_parsers&& ... rest)
->parse_result<std::tuple<parse_result_value_t<first_parser>, parse_result_value_t<rest_parsers>...>> {

	auto parse_result_ = std::invoke(f, s);
	if (!parse_result_)
		return parse_fail(std::move(parse_result_));

	auto parse_result_2 = parse_consecutive(parse_result_.rest(), std::forward<rest_parsers>(rest)...);
	if (!parse_result_2)
		return parse_fail(std::move(parse_result_2));

	return parse_result(
		std::tuple_cat(
			std::make_tuple(std::move(parse_result_.value())), std::move(parse_result_2.value())
		),
		parse_result_2.rest()
	);
}

template<typename first_parser>
constexpr auto parse_consecutive(const std::string_view s, first_parser&& f)
->parse_result<std::tuple<parse_result_value_t<first_parser>>> {
	auto t = std::invoke(f, s);
	if (!t)
		return parse_fail(std::move(t));
	return parse_result(std::make_tuple(std::move(t.value())), t.rest());
}

//parses using 2 parsers, one after each other, then joins them
template<typename parser1, typename parser2, typename join_fn>
constexpr auto parse_consecutive_and_join(const std::string_view s, parser1&& p1, parser2&& p2, join_fn&& fn)
->parse_result<std::invoke_result_t<join_fn, parse_result_value_t<parser1>, parse_result_value_t<parser2>>> {
	auto t = p1(s);
	if (!t)
		return parse_fail();
	auto o = p2(t.rest());
	if (!o)
		return parse_fail();
	return parse_result(std::invoke(fn, std::forward<decltype(t.value())>(t.value()), std::forward<decltype(o.value())>(o.value())), o.rest());
}

template<typename ...rest_parsers, size_t ...i>
constexpr auto parse_multi_consecutive2_impl(std::string_view s, std::index_sequence<i...>, rest_parsers&& ... rest)
->parse_result<std::tuple<parse_result_value_t<rest_parsers>...>> {
	try {
		auto do_thing = [&](auto&& parser) {
			auto pr = std::invoke(parser, s);
			if (pr) {
				s = pr.rest();
				return std::move(pr.value());
			}
			else {
				throw 1;
			}
		};
		//optional makes things default constructable
		std::tuple<std::optional<parse_result_value_t<rest_parsers>>...> hmm;
		((std::get<i>(hmm) = do_thing(rest)), ...);
		return parse_result(std::make_tuple(*std::get<i>(hmm)...), s);
	}
	catch (...) {
		return parse_fail();
	}
}

template<typename ...rest_parsers>
constexpr auto parse_multi_consecutive2(std::string_view s, rest_parsers&& ... rest)
->parse_result<std::tuple<parse_result_value_t<rest_parsers>...>> {
	return parse_multi_consecutive2_impl(s, std::index_sequence_for<rest_parsers...>(), std::forward<rest_parsers>(rest)...);
}

template<typename ...T>
struct multi_parser {
	constexpr multi_parser() = default;

	constexpr explicit multi_parser(T ... stuff) :
		m_parsers(std::forward<T>(stuff)...) {}

	constexpr decltype(auto) operator()(std::string_view s) {
		return std::apply([&](auto&& ...a) {
			return parse_consecutive(s, std::forward<decltype(a)>(a)...);
			},
			m_parsers);
	}

private:
	std::tuple<T...> m_parsers;
};

template<typename ...T>
multi_parser(T&&...)->multi_parser<T...>;

template<typename ...T>
struct try_multi_parser {
	constexpr try_multi_parser() = default;

	constexpr explicit try_multi_parser(T ... stuff) :
		m_parsers(std::move(stuff)...) {}

	constexpr decltype(auto) operator()(std::string_view s) {
		return std::apply([&](auto&& ...a) {
			return try_parse_multiple(s, std::forward<decltype(a)>(a)...);
			},
			m_parsers);
	}

private:
	std::tuple<T...> m_parsers;
};

template<typename ...T>
try_multi_parser(T&&...)->try_multi_parser<T...>;
