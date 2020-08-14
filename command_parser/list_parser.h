#pragma once
#include "parse_result.h"


//this might be meh ;-;
//parses thigns untill false
template<typename ...inner_parsers>
struct list_parser {
	using result_tuple = std::tuple<parse_result_value_t<inner_parsers>...>;

	explicit list_parser(inner_parsers ... a) :
		m_parsers(std::forward<inner_parsers>(a)...) { }

	parse_result<std::vector<result_tuple>> operator()(std::string_view s) {
		std::vector<result_tuple> ret;
		while (true) {
			//so i can bind the first arg of parse_consecutive to s
			auto rename_later = [&](auto&&... parsers) {
				return parse_consecutive(s, parsers...);
			};

			parse_result r = std::apply(rename_later, m_parsers);
			if (!r) {
				break;
			}
			else {
				ret.push_back(r.value());
				s = r.rest();
			}
		}
		return parse_result(std::move(ret), s);
	}

private:
	std::tuple<inner_parsers...> m_parsers;
};

template<typename ... inner_parsers>
list_parser(inner_parsers&&...)->list_parser<inner_parsers...>;


//same as list_parser, but only takes 1 parser and doesn't return a tuple
template<typename inner_parser>
struct list_parser1 {
	using result = parse_result_value_t<inner_parser>;

	explicit list_parser1(inner_parser a) :
		m_inner_parser(std::forward<inner_parser>(a)) { }

	parse_result<std::vector<result>> operator()(std::string_view s) {
		std::vector<result> ret;
		while (true) {

			parse_result r = m_inner_parser(s);

			if (!r) {
				break;
			}
			else {
				ret.push_back(r.value());
				s = r.rest();
			}
		}
		return parse_result(std::move(ret), s);
	}

private:
	//std::tuple<inner_parsers...> m_parsers;
	inner_parser m_inner_parser;
};

template<typename inner_parsers>
list_parser1(inner_parsers&&)->list_parser1<inner_parsers>;


//parses with delimiter and terminator
template<typename Inner_parser, typename Delimiter_parser, typename Terminal_parser>
struct list_parser2 {

	using inner_type = parse_result_value_t<Inner_parser>;

	explicit list_parser2(Inner_parser i, Delimiter_parser d, Terminal_parser t) :
		inner_parser(std::forward<Inner_parser>(i)),
		delimiter_parser(std::forward<Delimiter_parser>(d)),
		terminal_parser(std::forward<Terminal_parser>(t)) {};

	parse_result<std::vector<inner_type>> operator()(std::string_view s) {
		std::vector<inner_type> ret;
		{
			//check if it starts with thingy that ends it
			auto result1 = terminal_parser(s);
			if(result1) {
				return parse_result(std::move(ret), result1.rest());
			}
		}

		while (true) {
			auto result = parse_consecutive(
				s,
				inner_parser,
				first_of_parser(terminal_parser, delimiter_parser)
			);
			if (!result) {
				return parse_fail();
			}

			auto& [item, rawr] = result.value();
			s = result.rest();
			ret.push_back(std::move(item));

			if (rawr.index() == 0) {//parsed terminal_parser first
				return parse_result(std::move(ret), s);
			}
		}

	}

	Inner_parser inner_parser;
	Delimiter_parser delimiter_parser;
	Terminal_parser terminal_parser;
};

//parses with delimiter
template<typename Inner_parser, typename Delimiter_parser>
struct list_parser3 {

	using inner_type = parse_result_value_t<Inner_parser>;

	explicit list_parser3(Inner_parser i, Delimiter_parser d) :
		inner_parser(std::forward<Inner_parser>(i)),
		delimiter_parser(std::forward<Delimiter_parser>(d)) {};

	parse_result<std::vector<inner_type>> operator()(std::string_view s) {
		std::vector<inner_type> ret;

		while (true) {
			auto result = parse_consecutive(
				s,
				inner_parser,
				delimiter_parser
			);
			if (!result) {
				break;
			}

			auto& [item, rawr] = result.value();
			s = result.rest();
			ret.push_back(std::move(item));
		}

		return parse_result(ret, s);
	}

	Inner_parser inner_parser;
	Delimiter_parser delimiter_parser;
};

//delimiter + terminal, returns terminal
template<typename Inner_parser, typename Delimiter_parser, typename Terminal_parser>
struct list_parser4 {

	using inner_type = parse_result_value_t<Inner_parser>;

	explicit list_parser4(Inner_parser i, Delimiter_parser d, Terminal_parser t) :
		inner_parser(std::forward<Inner_parser>(i)),
		delimiter_parser(std::forward<Delimiter_parser>(d)),
		terminal_parser(std::forward<Terminal_parser>(t)) {};

	parse_result<std::pair<std::vector<inner_type>, parse_result_value_t<Terminal_parser>>> operator()(std::string_view s) {
		std::vector<inner_type> ret;
		{
			//check if it starts with thingy that ends it
			auto result1 = terminal_parser(s);
			if (result1) {
				return parse_result(std::make_pair(std::move(ret),result1.value()), result1.rest());
			}
		}

		while (true) {
			auto result = parse_consecutive(
				s,
				inner_parser,
				first_of_parser(terminal_parser, delimiter_parser)
			);

			if (!result) {
				return parse_fail();
			}

			auto& [item, terminal_or_delimiter] = result.value();
			s = result.rest();
			ret.push_back(std::move(item));
			if (terminal_or_delimiter.index() == 0) {//terminal
				return parse_result(std::make_pair(std::move(ret), terminal_or_delimiter.template get<0>()), s);
			}
		}
	}

	Inner_parser inner_parser;
	Delimiter_parser delimiter_parser;
	Terminal_parser terminal_parser;
};




