#pragma once
#include "parse_result.h"
#include "useful_parsers.h"
#include "useful_parsing_stuff.h"


template<typename... parsers>
struct combined_parser {
	
	explicit combined_parser(parsers... p):parser(std::forward<parsers>(p)...){}
	
	constexpr decltype(auto) operator()(std::string_view s) {
		return std::apply([&](auto&& ...a) {
			return parse_consecutive(s, std::forward<decltype(a)>(a)...);
		}, parser);
	}
	
	std::tuple<parsers...> parser;
};

template<typename...parsers>
struct combined_parsers_ignore_whitespace {
	explicit combined_parsers_ignore_whitespace(parsers... p) :parser(std::forward<parsers>(p)...) {}

	constexpr decltype(auto) operator()(std::string_view s) {
		return std::apply([&](auto&& ...a) {
			return parse_consecutive(s, transform_parser(multi_parser(whitespace_parser,std::forward<decltype(a)>(a)),ignore_left())...);
		}, parser);
	}

	std::tuple<parsers...> parser;
};

//stuff:
//'a' + b + c
//a + 'b' + "qwe"
//
//
//
//
//

//clang-format off

template<parser P1, parser P2> 
combined_parser<P1,P2> operator+(P1&& p1,P2&& p2) {
	return combined_parser(std::forward<P1>(p1), std::forward<P2>(p2));
}

template<parser P2, parser... P>
combined_parser<P..., P2> operator+(combined_parser<P...>&& p1, P2&& p2) {
	return std::apply([&](auto&&... parsers) {
		return combined_parser(std::move(parsers)..., std::forward<P2>(p2));
	},std::move(p1.parser));
}

template<parser P2, parser... P>
combined_parser<P..., P2> operator+(combined_parser<P...>& p1, P2&& p2) {
	return std::apply([&](auto&&... parsers) {
		return combined_parser(parsers..., std::forward<P2>(p2));
	}, p1.parser);
}

template<parser P2, parser... P>
combined_parser<P..., P2> operator+(const combined_parser<P...>& p1, P2&& p2) {
	return std::apply([&](auto&&... parsers) {
		return combined_parser(parsers..., std::forward<P2>(p2));
	}, p1.parser);
}


template<parser P2, parser... P>
combined_parser<P..., P2> operator+(const combined_parser<P...>&& p1, P2&& p2) {
	return std::apply([&](auto&&... parsers) {
		return combined_parser(parsers..., std::forward<P2>(p2));
	}, p1.parser);
}


//combined 2nd
template<parser P1, parser... P> 
combined_parser<P1, P...>  operator+(P1&& p1,const combined_parser<P...>& p2) {
	return std::apply([&](auto&&... parsers) {
		return combined_parser(std::forward<P1>(p1),parsers...);
	}, p2.parser);
}


template<parser P1, parser... P>
combined_parser<P1, P...>  operator+(P1&& p1, combined_parser<P...>&& p2) {
	return std::apply([&](auto&&... parsers) {
		return combined_parser(std::forward<P1>(p1), std::move(parsers)...);
	}, std::move(p2.parser));
}


template<parser P1, parser... P>
combined_parser<P1, P...>  operator+(P1&& p1, combined_parser<P...>& p2) {
	return std::apply([&](auto&&... parsers) {
		return combined_parser(std::forward<P1>(p1), parsers...);
		}, p2.parser);
}


template<parser P1, parser... P>
combined_parser<P1,P...> operator+(P1&& p1, const combined_parser<P...>&& p2) {
	return std::apply([&](auto&&... parsers) {
		return combined_parser(std::forward<P1>(p1), parsers...);
	}, p2.parser);
}

//char parsers
template<parser... P>
combined_parser<char_parser<true>, P...> operator+(char c, const combined_parser<P...>&& p2) {
	return std::apply([&](auto&&... parsers) {
		return combined_parser(char_parser(c), parsers...);
	}, p2.parser);
}

template<parser... P>
combined_parser<char_parser<true>, P...> operator+(char c, const combined_parser<P...>& p2) {
	return std::apply([&](auto&&... parsers) {
		return combined_parser(char_parser(c), parsers...);
		}, p2.parser);
}

template<parser... P>
combined_parser<char_parser<true>, P...> operator+(char c, combined_parser<P...>&& p2) {
	return std::apply([&](auto&&... parsers) {
		return combined_parser(char_parser(c), std::move(parsers)...);
		}, std::move(p2.parser));
}

template<parser... P>
combined_parser<char_parser<true>, P...> operator+(char c, combined_parser<P...>& p2) {
	return std::apply([&](auto&&... parsers) {
		return combined_parser(char_parser(c), parsers...);
		}, p2.parser);
}

//char second
template<parser... P>
combined_parser<P..., char_parser<true>> operator+(const combined_parser<P...>&& p2,char c) {
	return std::apply([&](auto&&... parsers) {
		return combined_parser(char_parser(c), parsers...);
	}, p2.parser);
}

template<parser... P>
combined_parser<P..., char_parser<true>> operator+(combined_parser<P...>&& p2, char c) {
	return std::apply([&](auto&&... parsers) {
		return combined_parser(std::move(parsers)..., char_parser(c));
	}, std::move(p2.parser));
}

template<parser... P>
combined_parser<P..., char_parser<true>> operator+(combined_parser<P...>& p2, char c) {
	return std::apply([&](auto&&... parsers) {
		return combined_parser(parsers..., char_parser(c));
	}, p2.parser);
}

template<parser... P>
combined_parser<P..., char_parser<true>> operator+(const combined_parser<P...>& p2, char c) {
	return std::apply([&](auto&&... parsers) {
		return combined_parser(parsers..., char_parser(c));
	}, p2.parser);
}

template<parser P1>
combined_parser<P1,char_parser<true>> operator+(P1&& p1, char c) {
	return combined_parser(std::forward<P1>(p1), char_parser(c));
}

template<parser P1>
combined_parser<char_parser<true>,P1> operator+(char c,P1&& p1) {
	return combined_parser(char_parser(c),std::forward<P1>(p1));
}

//clang-format on
