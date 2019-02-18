#include <rename_later_5.h>
#include "task_completion_handler.h"
#include <boost/asio/experimental/co_spawn.hpp>

cerwy::task<void> rename_later_5::start_stuffs() {
	//auto token = boost::asio::experimental::;
	m_open.store(true, std::memory_order_relaxed);
	while (true) {
		auto thing_to_send = co_await m_queue.pop();
		auto [ec,n] = co_await m_socket.async_write(boost::asio::buffer(thing_to_send), use_task_return_tuple2);
		if(ec) {
			break;
		}
	}		
	m_open.store(false, std::memory_order_relaxed);
	co_return;
};