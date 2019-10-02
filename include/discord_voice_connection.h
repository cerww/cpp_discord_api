#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include "rename_later_5.h"
#include "timed_task_executor.h"
#include "ref_count_ptr.h"
#include "snowflake.h"
#include "web_socket_session_impl.h"
#include <fmt/core.h>
#include "task_completion_handler.h"
#include "voice_channel.h"


struct discord_voice_connection_impl:
	ref_counted
{

	explicit discord_voice_connection_impl(web_socket_session sock);

	discord_voice_connection_impl(const discord_voice_connection_impl&) = delete;
	discord_voice_connection_impl(discord_voice_connection_impl&&) = delete;

	discord_voice_connection_impl& operator=(discord_voice_connection_impl&&) = delete;

	discord_voice_connection_impl& operator=(const discord_voice_connection_impl&) = delete;

	~discord_voice_connection_impl() = default;

	snowflake channel_id;
	snowflake my_id;
	snowflake guild_id;

	std::string token;
	std::string endpoint;
	std::string session_id;

	timed_task_executor* heartbeat_sender = nullptr;

	web_socket_session socket;

	int heartbeat_interval = 0;
	int ssrc = 0;
	
	union {		
		const voice_channel* channel;

		//used in setting up vc only
		cerwy::promise<void>* waiter = nullptr;
		//only 1 of these will be used at any time ;-;
	};

	
	int delay = 0;
	std::atomic<bool> is_alive = true;

	boost::asio::ip::udp::socket voice_socket;
	
	void start() {
		send_identify();
		socket.start_reads();
	}

	decltype(auto) ioc() {
		return socket.socket().get_executor();
	}


	cerwy::task<void> control_speaking(bool is_speaking = true) {		
		std::string msg = fmt::format(R"(
{
    "op": 5,
    "d": {
        "speaking": {},
        "delay": {},
        "ssrc": 1
    }
}
)",is_speaking,delay);

		co_await socket.send_thing(std::move(msg));

	}

	cerwy::task<void> send_silent_frames();
	
	void close() {
		is_alive = false;
		socket.close(1000);
	}

	cerwy::task<void> send_voice();

private:
	std::string m_ip;
	int m_port = 0;
	int m_hb_number = 1;
	std::vector<std::string> m_modes;

	std::vector<int> m_secret_key;

	cerwy::task<void> send_heartbeat();

	void on_msg_recv(nlohmann::json data,int opcode) {
		constexpr int ready = 2;
		constexpr int session_discription = 4;
		constexpr int speaking = 5;
		constexpr int heartbeat_ack = 6;
		constexpr int hello = 8;
		constexpr int resumed = 9;
		constexpr int disconnect = 13;

		switch(opcode) {
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

	void on_hello(nlohmann::json d);

	void send_identify();

	void on_ready(nlohmann::json data);

	void send_op1_select_protocol();

	void on_session_discription(nlohmann::json data);

	cerwy::task<void> connect_udp();

};

struct voice_connection{
	voice_connection() = default;
	
	voice_connection(const voice_connection&) = delete;
	voice_connection& operator=(const voice_connection&) = delete;

	voice_connection(voice_connection&&) = default;
	voice_connection& operator=(voice_connection&&) = default;

	~voice_connection() {
		disconnect();
	};

	explicit voice_connection(ref_count_ptr<discord_voice_connection_impl> connection):
		m_connection(std::move(connection)){}

	void disconnect() {
		m_connection->close();
		m_connection = nullptr;
	}

	cerwy::task<void> send() {	
		return m_connection->send_voice();
	};
	
private:
	ref_count_ptr<discord_voice_connection_impl> m_connection = nullptr;
};

