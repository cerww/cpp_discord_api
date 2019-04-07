#include <rename_later_5.h>
#include "task_completion_handler.h"

cerwy::task<void> rename_later_5::start_stuffs() {
	m_open = true;

	while(!m_tossed_away.empty()) {
		auto[ec, n] = co_await m_socket.async_write(boost::asio::buffer(m_tossed_away.front()), use_task_return_tuple2);
		if (ec) {
			goto die;
		}
		m_tossed_away.erase(m_tossed_away.begin());
	}
	while (true) {
		auto thing_to_send = co_await m_queue.pop();
		auto[ec, n] = co_await m_socket.async_write(boost::asio::buffer(thing_to_send), use_task_return_tuple2);
		if (ec) {
			m_tossed_away.push_back(std::move(thing_to_send));
			break;
		}
	}
	die:

	m_open = false;
};

