#include "web_socket_session_impl.h"
#include "web_socket_helpers.h"
#include <range/v3/all.hpp>
#include <utility>
#include <span>


cerwy::eager_task<void> web_socket_session_impl::reconnect(std::string uri) {
	m_buffer.consume(m_buffer.size());
	//m_socket = co_await wss_from_uri(uri, *m_resolver, *m_ssl_ctx);
	boost::asio::ip::tcp::resolver resolver(m_socket.get_executor());
	co_await reconnect_wss_from_url(m_socket, uri, resolver, m_ssl_ctx);
	//m_is_reading = false;
}

cerwy::eager_task<void> web_socket_session_impl::connect(std::string uri) {
	boost::asio::ip::tcp::resolver resolver(m_socket.get_executor());
	co_await reconnect_wss_from_url(m_socket, uri, resolver, m_ssl_ctx);
}

cerwy::eager_task<void> web_socket_session_impl::send_thing(const std::string msg) {
	auto pin = ref_count_ptr(this);

	auto lock = co_await m_mut.async_lock();
	auto [ec,n] = co_await m_socket.async_write(boost::asio::buffer(msg), use_task_return_tuple2);
	while (ec && ec.value() != boost::asio::error::operation_aborted && ec != boost::beast::websocket::error::closed) {		
		//TODO: change this
		boost::asio::steady_timer timer(m_socket.get_executor());
		timer.expires_from_now(std::chrono::seconds(5));
		auto [a] = co_await timer.async_wait(use_task_return_tuple2);
		std::tie(ec, n) = co_await m_socket.async_write(boost::asio::buffer(msg), use_task_return_tuple2);
	}
}

cerwy::eager_task<void> web_socket_session_impl::send_thing(const std::vector<std::byte> msg) {
	auto pin = ref_count_ptr(this);

	auto lock = co_await m_mut.async_lock();
	auto [ec, n] = co_await m_socket.async_write(boost::asio::buffer(msg), use_task_return_tuple2);
	while (ec && ec != boost::asio::error::operation_aborted) {
		//TODO: change this
		boost::asio::steady_timer timer(m_socket.get_executor());
		timer.expires_from_now(std::chrono::seconds(5));
		auto [a] = co_await timer.async_wait(use_task_return_tuple2);
		std::tie(ec, n) = co_await m_socket.async_write(boost::asio::buffer(msg), use_task_return_tuple2);
	}
}

static const auto nothing = [](auto&&...) {};

void web_socket_session_impl::close(int code) {
	m_socket.async_close(boost::beast::websocket::close_reason(code), nothing);
	m_is_alive = false;
}

cerwy::eager_task<void> web_socket_session_impl::start_reads() {
	auto pin = ref_count_ptr(this);
	if (m_is_reading.exchange(true)) {
		co_return;
	}

	while (m_socket.is_open()) {
		auto [ec, n] = co_await m_socket.async_read(m_buffer, use_task_return_tuple2);
		if (ec) {
			m_buffer.consume(n);
			co_await on_error(ec);
		} else {
			auto data = m_buffer.data();
			if (m_socket.got_text()) {
				auto str =
						data |
						ranges::views::transform([](auto a) { return std::string_view((const char*)a.data(), a.size()); }) |
						ranges::views::join |
						ranges::to<std::string>();
				m_buffer.consume(n);

				on_read(std::move(str));
			} else {//got binary
				auto str =
						data |
						ranges::views::transform([](auto a) { return std::span((std::byte*)a.data(), a.size()); }) |
						ranges::views::join |
						ranges::to<std::vector>();
				m_buffer.consume(n);

				on_binary(std::move(str));
			}
		}
	}
	m_is_reading = false;
}

void web_socket_session_impl::kill_me() {
	m_socket.async_close(boost::beast::websocket::close_reason(4000), nothing);
	//m_socket.close();
	m_is_alive = false;
}

cerwy::eager_task<web_socket_session> create_session(
	std::string_view uri,
	boost::asio::io_context& ioc,//TODO change this to any_io_executor or executor, watever is in boost 1.7.4
	boost::asio::ssl::context_base::method c
) {
	boost::asio::ip::tcp::resolver resolver(ioc);

	boost::asio::ssl::context ssl_ctx(c);

	auto sock = co_await wss_from_uri(uri, resolver, ssl_ctx);

	co_return web_socket_session(
		make_ref_count_ptr<web_socket_session_impl>(
			std::move(sock),
			std::move(ssl_ctx)
		)
	);
}
