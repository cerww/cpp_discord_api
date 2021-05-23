#pragma once
#include "eager_task.h"
#include "task_completion_handler.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "higher_order_functions.h"
#include <cctype>
#include <boost/beast/ssl.hpp>


cerwy::eager_task<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> ws_from_uri(std::string_view full_uri, boost::asio::ip::tcp::resolver& resolver);

cerwy::eager_task<boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>> wss_from_uri(
	std::string_view full_uri,
	boost::asio::ip::tcp::resolver& resolver,
	boost::asio::ssl::context& ssl_ctx
);

cerwy::eager_task<void> reconnect_wss_from_url(
	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>& socket,
	std::string_view full_uri,
	boost::asio::ip::tcp::resolver& resolver,
	boost::asio::ssl::context& ssl_ctx
);
