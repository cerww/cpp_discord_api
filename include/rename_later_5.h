#pragma once
#include <concurrent_coro_queue.h>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <ssl_stream.hpp>

//TODO, remove this along with create_shard, once i can
struct rename_later_5{
	rename_later_5(boost::beast::websocket::stream<ssl_stream<boost::asio::ip::tcp::socket>>& sock):m_socket(sock) {
		start_stuffs();
	}

	cerwy::task<void> start_stuffs();
	void send_thing(std::string s) {
		m_queue.push(std::move(s));
	}

	void close(int code) {
		m_socket.async_close(boost::beast::websocket::close_reason(code), [](auto&&...) {});
	}

	void maybe_restart() {
		if (!m_open) {
			start_stuffs();
		}
	}
	void reconnect() {
		
	}
private:
	boost::beast::websocket::stream<ssl_stream<boost::asio::ip::tcp::socket>>& m_socket;
	concurrent_coro_queue<std::string> m_queue;
	std::vector<std::string> m_tossed_away;
	std::atomic<bool> m_open = false;
};

