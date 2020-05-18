#pragma once
#include <functional>
#include <vector>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include "task.h"
#include "concurrent_async_queue.h"
#include "async_mutex.h"


struct web_socket_session_impl :ref_counted {
	web_socket_session_impl() = default;

	web_socket_session_impl(
		boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>> s,
		boost::asio::ip::tcp::resolver& resolver,
		boost::asio::ssl::context& ssl_ctx
	):
		m_socket(std::move(s)),
		m_resolver(&resolver),
		m_ssl_ctx(&ssl_ctx) {};

	std::function<void(std::string)> on_read;
	std::function<void(boost::beast::error_code)> on_error;

	cerwy::task<void> reconnect(std::string uri);
	cerwy::task<void> connect(std::string uri);

	cerwy::task<void> send_thing(std::string);

	void close(int);

	cerwy::task<void> start_reads();

	bool is_reading() const {
		return m_is_reading;
	}

	void kill_me();

	auto& socket() {
		return m_socket;
	}

	const auto& socket() const {
		return m_socket;
	}

	bool is_open() const {
		return m_socket.is_open();
	}

private:

	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>> m_socket;
	boost::beast::multi_buffer m_buffer;

	async_mutex m_mut;

	boost::asio::ip::tcp::resolver* m_resolver = nullptr;
	boost::asio::ssl::context* m_ssl_ctx = nullptr;

	std::atomic<bool> m_is_reading = false;

	std::atomic<bool> m_is_alive = true;
};

struct web_socket_session {
	
	//idk if this is safe to do ;-;
	web_socket_session() = default;

	web_socket_session(ref_count_ptr<web_socket_session_impl> me):
		m_me(std::move(me)) {}

	web_socket_session(const web_socket_session&) = delete;

	web_socket_session& operator=(const web_socket_session&) = delete;

	web_socket_session(web_socket_session&&) = default;
	web_socket_session& operator=(web_socket_session&&) = default;

	~web_socket_session() noexcept {
		if (m_me)
			m_me->close(1000);
	}

	std::function<void(std::string)>& on_read() {
		return m_me->on_read;
	};

	std::function<void(boost::beast::error_code)>& on_error() {
		return m_me->on_error;
	};

	const std::function<void(std::string)>& on_read() const {
		return m_me->on_read;
	};

	const std::function<void(boost::beast::error_code)>& on_error() const {
		return m_me->on_error;
	};

	cerwy::task<void> reconnect(std::string uri) {
		return m_me->reconnect(std::move(uri));
	};

	/*
	cerwy::task<void> connect(std::string uri) {		
		return m_me->connect(std::move(uri));		
	}
	
	*/

	cerwy::task<void> send_thing(std::string what) {
		return m_me->send_thing(std::move(what));
	};

	template<typename fn>
	void send_thing(std::string msg, fn&& f) {
		m_me->send_thing(std::move(msg), std::forward<fn>(f));
	}

	void close(int code) {
		m_me->close(code);
	}

	void start_reads() {
		m_me->start_reads();
	}

	bool is_reading() const {
		return m_me->is_reading();
	}

	auto& socket() {
		return m_me->socket();
	}

	const auto& socket() const {
		return m_me->socket();
	}

	bool is_open() const {
		return m_me->is_open();
	}

private:
	ref_count_ptr<web_socket_session_impl> m_me = nullptr;;
};

cerwy::task<web_socket_session> create_session(
	std::string_view full_uri,
	boost::asio::ip::tcp::resolver& resolver,
	boost::asio::ssl::context& ssl_ctx
);
