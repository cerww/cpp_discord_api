#pragma once
#include "web_socket_helpers.h"
#include "parsing_stuff.h"
#include <charconv>


namespace rawr {
	constexpr bool is_number(char c) noexcept {
		return c >= '0' && c <= '9';
	}

	constexpr bool is_letter(char c) noexcept {
		return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
	}
	constexpr std::tuple<std::string_view, std::string_view, std::string_view> parse_uri(std::string_view uri) {
		const auto split_point = uri.find_first_of(":/\\");
		const auto port = uri.substr(0, split_point);
		uri.remove_prefix(split_point);
		uri.remove_prefix(std::distance(uri.begin(), ranges::find_if(uri, hof::logical_disjunction(is_number, is_letter))));

		std::string_view path = uri.substr(std::min(uri.find('/'), uri.size() - 1));
		uri.remove_suffix(path.size());

		return { port,uri, path };
	}

	struct url_port_parser{
		parse_result<int> operator()(std::string_view part) {
			if(part.empty() || part[0] !=':') {
				return std::nullopt;
			}
			part.remove_prefix(1);
			const auto end_of_port = part.find_first_not_of("1234567890");
			if(end_of_port == 0) {
				return std::nullopt;
			}
			int port = 0;
			std::from_chars(part.data(), part.data() + end_of_port, port);
			part.remove_prefix(end_of_port);
			return parse_result(port, part);
		}
	};

	std::tuple<std::string_view, std::string_view, std::optional<int>, std::string_view> parse_url_better(std::string_view url) {
		auto parse_result = parse_multi_consecutive(
			url,
			parse_until([](char c) {return c==':'; }),
			str_parser<true>("://"),
			parse_until([](char c) {return c == ':' || c == '/' || c == '\\'; }),
			optional_parser(url_port_parser())
			//rest is the path
			);
		if(!parse_result) {
			throw std::runtime_error("wat");
		}
		auto [service, _, domain, port] = parse_result.value();
		auto path = parse_result.rest();
		return std::tuple(service,domain,port,path);
	}

}

cerwy::task<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> ws_from_uri(std::string_view full_uri, boost::asio::ip::tcp::resolver& resolver) {
	auto[port, uri, path] = rawr::parse_uri(full_uri);

	auto[ec, results] = co_await resolver.async_resolve(uri, port, use_task_return_tuple2);
	if (ec) {
		throw std::runtime_error(";-;");
	}
	boost::asio::ip::tcp::socket socket(resolver.get_executor());
	for (auto&& r : results) {
		socket.open(r.endpoint().protocol());
		socket.set_option(boost::asio::ip::tcp::no_delay(true));
		auto ec2 = co_await socket.async_connect(r.endpoint(), use_task_return_ec);
		if (!ec2) {
			break;
		}
		socket.close();
	}
	if (!socket.is_open()) {
		throw std::runtime_error(";-;");
	}

	boost::beast::websocket::stream<boost::asio::ip::tcp::socket> web_socket(std::move(socket));
	ec = co_await web_socket.async_handshake(std::string(uri), std::string(path), use_task_return_ec);
	if (ec) {
		throw std::runtime_error(";-;");
	}
	co_return std::move(web_socket);
}


cerwy::task<boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>> wss_from_uri(
	std::string_view full_uri,
	boost::asio::ip::tcp::resolver& resolver,
	boost::asio::ssl::context& ssl_ctx)
{
	auto[service, host, port,path] = rawr::parse_url_better(full_uri);

	if(service == "wss") {
		service = "https";
	}	

	auto [ec, results] = co_await resolver.async_resolve(host, false ? std::to_string(*port) : (std::string(service)), use_task_return_tuple2);

	if (ec) {
		throw std::runtime_error(";-;");
	}
	boost::asio::ip::tcp::socket socket(resolver.get_executor());

	auto[ec2,ep] = co_await boost::asio::async_connect(socket, results, use_task_return_tuple2);

	//co_await socket.async_connect(results,use_task_return_ec);
	//socket.
	//boost::asio::connect()
	if (!socket.is_open() || ec2) {
		throw std::runtime_error(";-;");
	}

	boost::beast::ssl_stream<boost::asio::ip::tcp::socket> ssl_socket(std::move(socket), ssl_ctx);
	ec = co_await ssl_socket.async_handshake(boost::asio::ssl::stream_base::handshake_type::client, use_task_return_ec);
	if(ec) {
		std::cout << ec.message() << std::endl;
		throw std::runtime_error(";-;");
	}
	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>> web_socket(std::move(ssl_socket));//;-;name so longgggggggg
	ec = co_await web_socket.async_handshake(std::string(host), std::string(path), use_task_return_ec);
	if (ec) {
		throw std::runtime_error(";-;");
	}
	co_return std::move(web_socket);
}

cerwy::task<void> reconnect_wss_from_url(
	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>& socket,
	std::string_view full_uri,
	boost::asio::ip::tcp::resolver& resolver,
	boost::asio::ssl::context& ssl_ctx) 
{
	auto [service, host, port, path] = rawr::parse_url_better(full_uri);

	if (service == "wss") {
		service = "https";
	}

	auto [ec, results] = co_await resolver.async_resolve(host, false ? std::to_string(*port) : (std::string(service)), use_task_return_tuple2);

	if (ec) {
		throw std::runtime_error(";-;");
	}

	auto [ec2, ep] = co_await boost::asio::async_connect(socket.next_layer().next_layer(), results, use_task_return_tuple2);

	if (!socket.next_layer().next_layer().is_open() || ec2) {
		std::cout << ec2.message()<<std::endl;
		throw std::runtime_error(";-;");
	}

	ec = co_await socket.next_layer().async_handshake(boost::asio::ssl::stream_base::handshake_type::client, use_task_return_ec);

	if (ec) {
		std::cout << ec.message() << std::endl;
		throw std::runtime_error(";-;");
	}
	ec = co_await socket.async_handshake(std::string(host), std::string(path), use_task_return_ec);
	if (ec) {
		throw std::runtime_error(";-;");
	}

}
