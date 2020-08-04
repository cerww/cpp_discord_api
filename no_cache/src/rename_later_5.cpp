#include "rename_later_5.h"
#include "../common/task_completion_handler.h"

namespace cacheless {

cerwy::task<void> rename_later_5::send_thing(std::string msg) {
	auto lock = co_await m_mut.async_lock();
	auto [ec, n] = co_await m_socket.async_write(boost::asio::buffer(msg), use_task_return_tuple2);
	//somehow make this different
	while (ec) {
		boost::asio::steady_timer timer(m_socket.get_executor(), std::chrono::steady_clock::now() + std::chrono::seconds(5));
		co_await timer.async_wait(use_task);
		std::tie(ec, n) = co_await m_socket.async_write(boost::asio::buffer(msg), use_task_return_tuple2);
	}
}

void rename_later_5::close(int code) noexcept {
	if (m_socket.is_open())
		m_socket.async_close(boost::beast::websocket::close_reason(code), [](auto&&...) {});
};
}
