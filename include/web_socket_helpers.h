#pragma once
#include "task.h"
#include "task_completion_handler.h"
#include "ssl_stream.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "higher_order_functions.h"
#include <range/v3/all.hpp>
#include <cctype>


namespace rawr{
constexpr bool is_number(char c) noexcept{
	return c >= '0' && c <= '9';
}

constexpr bool is_letter(char c) noexcept {
	return c >= 'a' && c <= 'z' || c>='A' && c<='Z';
}
constexpr std::tuple<std::string_view, std::string_view, std::string_view> parse_uri(std::string_view uri) {
	const auto split_point = uri.find_first_of(":/\\");
	const auto port = uri.substr(0, split_point);
	uri.remove_prefix(split_point);
	uri.remove_prefix(std::distance(uri.begin(), ranges::find_if(uri, hof::logical_disjunction(is_number,is_letter))));

	std::string_view path = uri.substr(std::min(uri.find('/'), uri.size() - 1));
	uri.remove_suffix(path.size());

	return { port,uri, path };
}
}

cerwy::task<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> ws_from_uri(std::string_view full_uri, boost::asio::ip::tcp::resolver& resolver);

cerwy::task<boost::beast::websocket::stream<ssl_stream<boost::asio::ip::tcp::socket>>> wss_from_uri(
	std::string_view full_uri,
	boost::asio::ip::tcp::resolver& resolver,
	boost::asio::ssl::context& ssl_ctx
);


