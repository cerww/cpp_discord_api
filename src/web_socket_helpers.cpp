#pragma once
#include "web_socket_helpers.h"

cerwy::task<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> ws_from_uri(std::string_view full_uri, boost::asio::ip::tcp::resolver& resolver) {
	auto[port, uri, path] = rawr::parse_uri(full_uri);

	auto[ec, results] = co_await resolver.async_resolve(uri, port, use_task_return_tuple2);
	if (ec) {
		//?????
	}
	boost::asio::ip::tcp::socket socket(resolver.get_io_context());
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

cerwy::task<boost::beast::websocket::stream<ssl_stream<boost::asio::ip::tcp::socket>>> wss_from_uri(
	std::string_view full_uri,
	boost::asio::ip::tcp::resolver& resolver,
	boost::asio::ssl::context& ssl_ctx)
{
	//auto[port, uri, path] = rawr::parse_uri(full_uri);
	//auto query = boost::asio::ip::tcp::resolver::query(std::string(uri), std::string(port));
	std::string_view port = "https";
	std::string_view uri = "gateway.discord.gg";
	std::string_view path = "/v=6&encoding=json";
	auto[ec, results] = co_await resolver.async_resolve(uri, port, use_task_return_tuple2);

	if (ec) {
		throw std::runtime_error(";-;");
	}
	boost::asio::ip::tcp::socket socket(resolver.get_io_context());
	for (auto&& r : results) {
		socket.open(r.endpoint().protocol());
		//socket.set_option(boost::asio::ip::tcp::no_delay(true));
		auto ec2 = co_await socket.async_connect(r.endpoint(), use_task_return_ec);
		if (!ec2) {
			break;
		}
		socket.close();
	}
	//boost::asio::connect()
	if (!socket.is_open()) {
		throw std::runtime_error(";-;");
	}

	ssl_stream<boost::asio::ip::tcp::socket> ssl_socket(std::move(socket), ssl_ctx);
	ec = co_await ssl_socket.async_handshake(boost::asio::ssl::stream_base::handshake_type::client, use_task_return_ec);

	boost::beast::websocket::stream<ssl_stream<boost::asio::ip::tcp::socket>> web_socket(std::move(ssl_socket));//;-;name so longgggggggg
	ec = co_await web_socket.async_handshake(std::string(uri), std::string(path), use_task_return_ec);
	if (ec) {
		throw std::runtime_error(";-;");
	}
	co_return std::move(web_socket);
}


