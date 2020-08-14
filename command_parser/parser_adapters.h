#pragma once
#include "parse_result.h"
#include "useful_parsers.h"


template<typename inner_parser>
struct ignore_leading_whitespace {
	
	explicit ignore_leading_whitespace(inner_parser i):parser(std::forward<inner_parser>(i)){}

	auto operator()(std::string_view s) {
		return parser(whitespace_parser(s).rest());
	}

	inner_parser parser;
};

template<typename inner_parser>
ignore_leading_whitespace(inner_parser&&)->ignore_leading_whitespace<inner_parser>;

template<typename Parser, typename fn>
auto transform_parser(Parser&& p, fn&& f) {
	using parse_value_type = parse_result_value_t<Parser>;
	static_assert(std::is_invocable_v<fn, parse_value_type>);
	
	struct transformed_parser {

		transformed_parser(Parser p, fn ff) :
			parser(std::forward<Parser>(p)),
			f(std::forward<fn>(ff)) {}

		auto operator()(std::string_view s) {
			return parser(s).transform(f);
		}

		Parser parser;
		fn f;
	};
	return transformed_parser(std::forward<Parser>(p), std::forward<fn>(f));
}


template<typename parser_t>
struct optional_parser {
	constexpr optional_parser() = default;

	constexpr explicit optional_parser(parser_t a) :
		parser(std::move(a)) {}

	constexpr parse_result<std::optional<parse_result_value_t<parser_t>>> operator()(std::string_view s) {
		auto t = parser(s);
		if (t) {
			return parse_result(std::optional(std::move(t.value())), t.rest());
		}
		else {
			return parse_result<std::optional<parse_result_value_t<parser_t>>>(std::nullopt, s);
		}
	}

	parser_t parser;
};

template<typename p>
optional_parser(p&&)->optional_parser<p>;
