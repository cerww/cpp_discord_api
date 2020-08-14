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
#include <cassert>
#include <charconv>
#include <vector>
#include "parse_result.h"
#include "useful_parsers.h"
#include "parse_first_of.h"
#include "list_parser.h"
#include "useful_parsing_stuff.h"

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

struct not_start_with {
	parse_result<bool> operator()(std::string_view s) const {
		if (!s.empty() && s.front() == c) {
			return parse_fail();
		}
		return parse_result(true, s);
	}

	char c;
};

template<typename pred>
struct not_start_with_pred {
	parse_result<bool> operator()(std::string_view s) {
		if (!s.empty() && std::invoke(p, s.front())) {
			return parse_fail();
		}
		return parse_result(true, s);
	}

	pred p;
};

struct ensures_not_empty {
	parse_result<bool> operator()(std::string_view s) const {
		if (s.empty()) {
			return parse_fail();
		}
		return parse_result(true, s);
	}
};

template<typename Pred>
struct ensures_start_with {

	explicit ensures_start_with(Pred a):
		pred(std::forward<Pred>(a)) {}


	parse_result<bool> operator()(std::string_view s) {
		if (!s.empty() && std::invoke(pred, s.front())) {
			return parse_fail();
		}
		return parse_result(true, s);
	}

	Pred pred;
};

template<typename p>
ensures_start_with(p&&)->ensures_start_with<p>;



