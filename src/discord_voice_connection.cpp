#include "discord_voice_connection.h"
#include <sodium.h>

discord_voice_connection_impl::discord_voice_connection_impl(web_socket_session sock):
	socket(std::move(sock)),
	voice_socket(socket.socket().get_executor()) {
	socket.on_read() = [this](std::string_view s) {
		auto json = nlohmann::json::parse(s);
		const int opcode = json["op"];
		on_msg_recv(std::move(json["d"]), opcode);
	};

	socket.on_error() = [this](boost::system::error_code ec) {
		if(ec!=boost::asio::error::operation_aborted)
			socket.reconnect(endpoint);
	};
}

cerwy::task<void> discord_voice_connection_impl::send_silent_frames() {
	//0xF8, 0xFF,0xFE
	co_return;
}

cerwy::task<void> discord_voice_connection_impl::send_voice() {
	//....idk wat to do ;-;
	
	
	co_return;
}

cerwy::task<void> discord_voice_connection_impl::send_heartbeat() {
	/*
	heartbeat_sender->execute([me = ref_count_ptr(this)]() {
		if(me->ref_count() > 1 &&me->socket.is_open()) {
			//send heartbeat
			std::string hb = R"(
			{
			"op":3,
			"d":1
			}
			)";
			me->socket.send_thing(std::move(hb));
			me->send_heartbeat();
		}else {
			if(me->socket.is_open())
				me->socket.close(1000);
		}
	}, std::chrono::steady_clock::now() + std::chrono::milliseconds(heartbeat_interval));
	*/
	auto pin = ref_count_ptr(this);
	while(is_alive) {
		boost::asio::steady_timer timer(ioc());
		timer.expires_after(std::chrono::milliseconds(heartbeat_interval));
		auto ec = co_await timer.async_wait(use_task_return_ec);
		if(ec) {
			break;
		}
		std::string hb = R"(
			{
			"op":3,
			"d":1
			}
			)";
		socket.send_thing(std::move(hb));
	}

	co_return;
}

void discord_voice_connection_impl::on_msg_recv(nlohmann::json data, int opcode) {
	constexpr int ready = 2;
	constexpr int session_discription = 4;
	constexpr int speaking = 5;
	constexpr int heartbeat_ack = 6;
	constexpr int hello = 8;
	constexpr int resumed = 9;
	constexpr int disconnect = 13;

	switch (opcode) {
	case ready:
		on_ready(std::move(data));
		break;
	case session_discription:
		on_session_discription(std::move(data));
		break;
	case speaking:
		break;
	case heartbeat_ack:
		//do nothing? ;-;
		break;
	case hello:
		on_hello(std::move(data));
		break;
	case resumed:
		break;
	case disconnect:
		break;
	default:
		//wat
		break;
	}
}

void discord_voice_connection_impl::on_hello(nlohmann::json d) {
	heartbeat_interval = (d["heartbeat_interval"].get<int>() * 3) / 4;
	std::string hb = R"(
				{
				"op":3,
				"d":1
				}
				)";
	socket.send_thing(std::move(hb));
	send_heartbeat();
}

void discord_voice_connection_impl::send_identify() {
	std::string thing = fmt::format(
		R"(

{{
    "op": 0,
    "d": {{
        "server_id": "{}",
        "user_id": "{}",
        "session_id": "{}",
        "token": "{}"
    }}
}}
)",
		guild_id.val, my_id.val, session_id, token);

	socket.send_thing(std::move(thing));
	//sodium_init();
}

void discord_voice_connection_impl::on_ready(nlohmann::json data) {
	ssrc = data["ssrc"].get<int>();
	m_ip = data["ip"].get<std::string>();
	m_port = data["port"].get<int>();
	m_modes = data["modes"].get<std::vector<std::string>>();
	send_op1_select_protocol();
}

void discord_voice_connection_impl::send_op1_select_protocol() {
	std::string msg = fmt::format(
		R"({{
			"op": 1,
			"d": {{
				"protocol": "udp",
				"data": {{
					"address": "{}",
					"port": {},
					"mode": "xsalsa20_poly1305"
				}}
			}}
		}})",
		m_ip, m_port);

	socket.send_thing(std::move(msg));
}

void discord_voice_connection_impl::on_session_discription(nlohmann::json data) {
	m_secret_key = data["secret_key"].get<std::vector<int>>();
	connect_udp();
}

cerwy::task<void> discord_voice_connection_impl::connect_udp() {
	auto resolver = boost::asio::ip::udp::resolver(ioc());
	auto [ec,results] = co_await resolver.async_resolve(m_ip, std::to_string(m_port), use_task_return_tuple2);
	if (ec) {
		//die?
		int u = 0;
	}

	auto [ec2,ep] = co_await boost::asio::async_connect(voice_socket, results, use_task_return_tuple2);
	if (ec2) {
		int y = 0;
	} else {
		waiter->set_value();
	}
}
