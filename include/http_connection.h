#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
//#include "concurrent_queue.h"
#include "../common/eager_task.h"
#include "../common/async_mutex.h"
#include "../common/concurrent_async_queue.h"
#include "../common/rate_limiter.h"
#include "discord_request.h"
#include "../common/lazy_task.h"
//#include <variant>

constexpr static int aushdkjasdjaskldasd = sizeof(boost::beast::http::request<boost::beast::http::string_body>);


struct http_connection2 {
	void sleep_till(std::chrono::system_clock::time_point time_point) {
		//this is thread safe?
		m_rate_limted_until = time_point;
		m_global_rate_limited.store(true);
	}

	http_connection2() = delete;
	http_connection2(http_connection2&&) = delete;
	http_connection2(const http_connection2&) = delete;
	http_connection2& operator=(const http_connection2&) = delete;
	http_connection2& operator=(http_connection2&&) = delete;

	explicit http_connection2(boost::asio::io_context&);

	~http_connection2() {
		m_done.store(true);
		m_request_queue.cancel_all();
	}

	void send(discord_request&& d);

	void stop() {
		m_done.store(true);
	}

	cerwy::eager_task<boost::beast::error_code> async_connect();

	void set_global_ratelimit_fn(std::function<void(std::chrono::system_clock::time_point)> f) {
		m_on_global_ratelimit = std::move(f);		
	}
	
private:
	std::chrono::system_clock::time_point m_rate_limted_until = {};
	std::atomic<bool> m_global_rate_limited = false;
	std::function<void(std::chrono::system_clock::time_point)> m_on_global_ratelimit = [](std::chrono::system_clock::time_point) {};

	std::atomic<bool> m_done = false;	

	boost::asio::io_context& m_ioc;
	boost::asio::ip::tcp::resolver m_resolver;

	boost::asio::ssl::context m_ssl_ctx{ boost::asio::ssl::context::tlsv12_client };
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_socket{ m_ioc, m_ssl_ctx };
	
	boost::beast::flat_buffer m_buffer{};
	//mpsc_concurrent_async_queue<discord_request> m_request_queue = {};
	async_queue_maybe_better<discord_request> m_request_queue = {};	

	cerwy::lazy_task<void> send_to_discord(discord_request);
	cerwy::lazy_task<void> send_to_discord(discord_request, uint64_t major_param_id_);
	cerwy::lazy_task<bool> send_to_discord_(discord_request&, boost::beast::http::response<boost::beast::http::string_body>&);
	cerwy::eager_task<void> start_sending();
	cerwy::lazy_task<void> reconnect();
	cerwy::lazy_task<void> send_rq(discord_request&,boost::beast::http::response<boost::beast::http::string_body>&);

	rate_limiter<uint64_t, discord_request, std::chrono::system_clock> m_rate_limiter;
};

static constexpr int audhaskjdasd = sizeof(std::function<void()>);

