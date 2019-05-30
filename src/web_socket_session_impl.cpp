#include "web_socket_session_impl.h"
#include "web_socket_helpers.h"
#include <range/v3/all.hpp>
#include <utility>

cerwy::task<void> web_socket_session_impl::reconnect(std::string uri){
	m_buffer.consume(m_buffer.size());
	//m_socket = co_await wss_from_uri(uri, *m_resolver, *m_ssl_ctx);
	co_await reconnect_wss_from_url(m_socket, uri, *m_resolver, *m_ssl_ctx);
	m_is_reading = false;
	m_is_writing = false;
}

cerwy::task<void> web_socket_session_impl::connect(std::string uri) {
	 co_await reconnect_wss_from_url(m_socket,uri, *m_resolver, *m_ssl_ctx);
}

void web_socket_session_impl::send_thing(std::string msg) {
	m_queue.push(std::make_pair(std::move(msg), std::nullopt));
}

void web_socket_session_impl::send_thing(std::string msg, std::function<void()> fn) {
	m_queue.push(std::make_pair(std::move(msg), std::move(fn)));
}

void web_socket_session_impl::close(int code) {
	m_socket.async_close(boost::beast::websocket::close_reason(code), [](auto && ...) {});
	m_queue.cancel_all();
}

cerwy::task<void> web_socket_session_impl::start_reads() {
	auto pin = ref_count_ptr(this);
	if(m_is_reading.exchange(true)) {
		co_return;
	}

	while(m_socket.is_open()) {
		auto [ec, n] = co_await m_socket.async_read(m_buffer, use_task_return_tuple2);
		if(!m_socket.is_open() || ec) {
			m_is_reading = false;
			on_error(ec);
		}else {
			auto data = m_buffer.data();
			auto str =
				data |
				ranges::view::transform([](auto a) {return std::string_view((const char*)a.data(), a.size()); }) |
				ranges::view::join |
				ranges::to_<std::string>();
			m_buffer.consume(n);

			on_read(std::move(str));
		}
	}
	die:
	m_is_reading = false;
}

cerwy::task<void> web_socket_session_impl::start_writes() {
	auto pin_me = ref_count_ptr(this);
	if(m_is_writing.exchange(true)) {
		co_return;
	}

	while(!m_tossed_away.empty()) {
		auto msg = std::move(m_tossed_away.front());
		m_tossed_away.erase(m_tossed_away.begin());
		auto [ec, n] = co_await m_socket.async_write(boost::asio::buffer(msg.first), use_task_return_tuple2);
		if(!m_socket.is_open()) {
			goto die;
		}else if(ec){
			m_tossed_away.insert(m_tossed_away.begin(), std::move(msg));
		}else if (msg.second) {
			std::invoke(*msg.second);
		}

	}
	while(m_socket.is_open()) {
		auto msg = co_await m_queue.pop();
		auto[ec,n] = co_await m_socket.async_write(boost::asio::buffer(msg.first),use_task_return_tuple2);
		if(ec) {
			if(!m_socket.is_open()) {
				goto die;
			}else {
				m_tossed_away.push_back(std::move(msg));
				goto die;
			}
		}else {
			if(msg.second) {
				std::invoke(*msg.second);
			}
		}
	}
	die:
	m_is_writing = false;
}

void web_socket_session_impl::kill_me() {
	m_socket.async_close(boost::beast::websocket::close_reason(4000), [](auto && ...) {});
	//m_socket.close();
	m_is_alive = false;	
}

cerwy::task<web_socket_session> create_session(
	std::string_view uri,
	boost::asio::ip::tcp::resolver& resolver,
	boost::asio::ssl::context& ssl_ctx
) {	
	co_return web_socket_session(
		make_ref_count_ptr<web_socket_session_impl>(
			co_await wss_from_uri(uri, resolver, ssl_ctx), 
			resolver, 
			ssl_ctx
		)
	);
}
