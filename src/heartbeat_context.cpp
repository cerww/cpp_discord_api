#include "heartbeat_context.h"
#include "internal_shard.h"
#include <fmt/core.h>

using namespace fmt::literals;

void heartbeat_context::start() {
	m_socket = m_shard.m_web_socket.get();
	
	std::string msg =
		R"({ "op": 1, "d" : null })";

	m_socket->send_thing(std::move(msg));

	recived_ack = false;

	auto t = std::make_unique<boost::asio::steady_timer>(m_shard.strand().context());

	t->expires_after(std::chrono::milliseconds(hb_interval));
	t->async_wait([this, temp = std::move(t)](auto ec){
		if (!ec)
			send_heartbeat();
	});
}



void heartbeat_context::send_heartbeat() {
	if (!recived_ack.load()) {
		//reconnect();
		m_shard.reconnect();
		return;
	}
	
	if (m_shard.is_disconnected())
		return;

	std::string msg =
		R"({{ "op": 1, "d" : {} }})"_format(m_shard.m_seq_num.load(std::memory_order_relaxed));
	
	m_socket->send_thing(std::move(msg));
	
	recived_ack = false;

	auto t = std::make_unique<boost::asio::steady_timer>(m_shard.strand().context());
	
	t->expires_after(std::chrono::milliseconds(hb_interval));
	t->async_wait([this, temp = std::move(t)](auto ec){
		if (!ec)
			send_heartbeat();
	});
	
}
