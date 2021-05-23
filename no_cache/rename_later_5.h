#pragma once
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include<boost/beast/ssl.hpp>
#include "../common/async_mutex.h"

//TODO, remove this with create_shard when i can
//things that queues writes to socket

namespace cacheless {

struct rename_later_5 {
	explicit rename_later_5(boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>& sock) :
		m_socket(sock) {

		//start_stuffs();
	}

	~rename_later_5() {
		*m_is_alive = false;
	}

	cerwy::eager_task<void> send_thing(std::string);

	void close(int code) noexcept;

	void maybe_restart() {
		//?????????????
	}

	void reconnect() {
		//?????????????
	}

private:
	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>& m_socket;
	std::shared_ptr<bool> m_is_alive = std::make_shared<bool>(true);
	async_mutex m_mut;
};
}
