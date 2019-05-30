#pragma once
#include <concurrent_async_queue.h>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include<boost/beast/ssl.hpp>
#include "async_mutex.h"

//TODO, remove this with create_shard when i can
struct rename_later_5{
	explicit rename_later_5(boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>& sock):m_socket(sock) {
		start_stuffs();
	}	

	~rename_later_5() {
		*m_is_alive = false;
	}

	cerwy::task<void> start_stuffs();

	void send_thing(std::string s) {
		send_thing2(std::move(s));
		//m_queue.push(std::move(s));
	}

	cerwy::task<void> send_thing2(std::string);

	void close(int code) noexcept;

	void maybe_restart() {
		if (!m_open) {
			start_stuffs();
		}
	}

	void reconnect() {
		//?????????????
	}

private:
	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>& m_socket;
	concurrent_async_queue<std::string> m_queue;
	std::vector<std::string> m_tossed_away;
	std::atomic<bool> m_open = true;
	std::shared_ptr<bool> m_is_alive = std::make_shared<bool>(true);
	async_mutex m_mut;

};

