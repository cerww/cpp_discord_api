#include "rename_later_5.h"
#include "task_completion_handler.h"

cerwy::task<void> rename_later_5::start_stuffs() {
	m_open = true;

	while(!m_tossed_away.empty()) {
		auto[ec, n] = co_await m_socket.async_write(boost::asio::buffer(m_tossed_away.front()), use_task_return_tuple2);
		if(!*m_is_alive) {
			co_return;
		}
		if (ec) {
			goto die;
		}
		m_tossed_away.erase(m_tossed_away.begin());
	}

	while (true) {
		auto thing_to_send = co_await m_queue.pop();
		if (!m_open) {
			co_return;
		}
		auto[ec, n] = co_await m_socket.async_write(boost::asio::buffer(thing_to_send), use_task_return_tuple2);

		if (!*m_is_alive) {
			co_return;
		}

		if (ec) {
			m_tossed_away.push_back(std::move(thing_to_send));
			break;
		}
	}
	die:

	m_open = false;
}

cerwy::task<void> rename_later_5::send_thing(std::string msg) {
	auto lock = co_await m_mut.async_lock();
	auto [ec, n] = co_await m_socket.async_write(boost::asio::buffer(msg), use_task_return_tuple2);
	while(ec) {
		boost::asio::steady_timer timer(m_socket.get_executor(), std::chrono::steady_clock::now() + std::chrono::seconds(5));
		co_await timer.async_wait(use_task);
		std::tie(ec, n) = co_await m_socket.async_write(boost::asio::buffer(msg), use_task_return_tuple2);
	}
}

void rename_later_5::close(int code) noexcept {
	if (m_socket.is_open()) m_socket.async_close(boost::beast::websocket::close_reason(code), [](auto&&...) {});
	m_open = false;
};

