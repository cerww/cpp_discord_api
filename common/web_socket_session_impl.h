#pragma once
#include <functional>
#include <vector>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include "eager_task.h"
#include "async_mutex.h"
#include <iostream>
//#include <ctime>


struct web_socket_session_impl :ref_counted {
	//web_socket_session_impl() = default;

	web_socket_session_impl(
		boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>> s,
		boost::asio::ssl::context ssl_ctx
	):
		m_socket(std::move(s)),
		m_ssl_ctx(std::move(ssl_ctx)) { };

	web_socket_session_impl(
		boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>> s,
		boost::asio::ssl::context_base::method ssl_ctx_method
	) :
		m_socket(std::move(s)),
		m_ssl_ctx(ssl_ctx_method) { };

	std::function<void(std::string)> on_read = [](auto&&...) {};
	std::function<void(std::vector<std::byte>)> on_binary = [](auto&&...) {};

	std::function<cerwy::eager_task<void>(boost::beast::error_code)> on_error = [](auto&&...) { return cerwy::make_ready_void_task(); };//reconsider task?

	cerwy::eager_task<void> reconnect(std::string uri);
	cerwy::eager_task<void> connect(std::string uri);

	cerwy::eager_task<void> send_thing(std::string);

	cerwy::eager_task<void> send_thing(std::vector<std::byte>);

	void close(int);

	cerwy::eager_task<void> start_reads();

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

	boost::asio::ssl::context m_ssl_ctx;

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

	std::function<void(std::vector<std::byte>)>& on_binary() {
		return m_me->on_binary;
	}

	std::function<cerwy::eager_task<void>(boost::beast::error_code)>& on_error() {
		return m_me->on_error;
	};

	const std::function<void(std::string)>& on_read() const {
		return m_me->on_read;
	};
	
	const std::function<void(std::vector<std::byte>)>& on_binary() const{
		return m_me->on_binary;
	}

	const std::function<cerwy::eager_task<void>(boost::beast::error_code)>& on_error() const {
		return m_me->on_error;
	};

	// ReSharper disable CppMemberFunctionMayBeConst
	cerwy::eager_task<void> reconnect(std::string uri) {
		// ReSharper restore CppMemberFunctionMayBeConst
		return m_me->reconnect(std::move(uri));
	};

	/*
	cerwy::eager_task<void> connect(std::string uri) {		
		return m_me->connect(std::move(uri));		
	}
	
	*/

	cerwy::eager_task<void> send_thing(std::string what) const {
		return m_me->send_thing(std::move(what));
	};

	cerwy::eager_task<void> send_thing(std::vector<std::byte> what) const {
		return m_me->send_thing(std::move(what));
	};

	template<typename fn>
	void send_thing(std::string msg, fn&& f) {
		m_me->send_thing(std::move(msg), std::forward<fn>(f));
	}

	// ReSharper disable CppMemberFunctionMayBeConst
	void close(int code) {
		// ReSharper restore CppMemberFunctionMayBeConst
		m_me->close(code);
	}

	// ReSharper disable CppMemberFunctionMayBeConst
	void start_reads() {
		// ReSharper restore CppMemberFunctionMayBeConst
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

cerwy::eager_task<web_socket_session> create_session(
	std::string_view full_uri,
	boost::asio::io_context& ioc,//make this any_io_executor?	
	boost::asio::ssl::context_base::method c
);
