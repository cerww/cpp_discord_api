#include "discord_voice_connection.h"
#define SODIUM_LIBRARY_MINIMAL
#include <sodium.h>
#include <iostream>
#include <fmt/core.h>
#include "../common/resume_on_strand.h"
#include "internal_shard.h"

discord_voice_connection_impl::discord_voice_connection_impl(web_socket_session sock,boost::asio::io_context& ioc):
	socket(std::move(sock)),
	voice_socket(ioc, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)) {
	
	socket.on_read() = [this](std::string_view s) {
		auto json = nlohmann::json::parse(s);
		const int opcode = json["op"];
		on_msg_recv(std::move(json["d"]), opcode);
	};

	socket.on_error() = [this](boost::system::error_code ec) ->cerwy::eager_task<void> {
		std::cout << "error voice websocket "<< ec << std::endl;	
		if (ec != boost::asio::error::operation_aborted) {
			if(ec.value() != 4014) {//check if its a websocket error too, idk how
				co_await socket.reconnect(web_socket_endpoint);				
			}
		}
		co_return;
	};
}

cerwy::eager_task<void> discord_voice_connection_impl::control_speaking(int is_speaking) {
	std::string msg = fmt::format(R"(
{{
    "op": 5,
    "d": {{
        "speaking": {},
        "delay": {},
        "ssrc": {}
    }}
}}
)", is_speaking, delay,ssrc);

	co_await socket.send_thing(std::move(msg));
}

cerwy::eager_task<void> discord_voice_connection_impl::send_silent_frames() {
	//0xF8, 0xFF,0xFE
	co_return;
}

void discord_voice_connection_impl::close() {
	is_alive = false;
	socket.close(1000);
	cancel_current_data();
	//shard_ptr->voice_connections.erase(guild_id);//this is bad, shard might be dead by now ;-;
}

using namespace std::literals;

/*
TODO: convert these to C++
	def _encrypt_xsalsa20_poly1305(self, header, data):
		box = nacl.secret.SecretBox(bytes(self.secret_key))
		nonce = bytearray(24)
		nonce[:12] = header

		return header + box.encrypt(bytes(data), bytes(nonce)).ciphertext

	def _encrypt_xsalsa20_poly1305_suffix(self, header, data):
		box = nacl.secret.SecretBox(bytes(self.secret_key))
		nonce = nacl.utils.random(nacl.secret.SecretBox.NONCE_SIZE)

		return header + box.encrypt(bytes(data), nonce).ciphertext + nonce

	def _encrypt_xsalsa20_poly1305_lite(self, header, data):
		box = nacl.secret.SecretBox(bytes(self.secret_key))
		nonce = bytearray(24)

		nonce[:4] = struct.pack('>I', self._lite_nonce)
		self.checked_add('_lite_nonce', 1, 4294967295)


 */

constexpr int a = crypto_secretbox_KEYBYTES;//32
constexpr int b = crypto_secretbox_NONCEBYTES;//24
constexpr int c = crypto_secretbox_MACBYTES;//16

std::vector<std::byte> discord_voice_connection_impl::encrypt_xsalsa20_poly1305(const std::array<std::byte, 12> header, std::span<const std::byte> audio_data) {
	//std::vector<std::byte> return_val;


	constexpr int header_size_bytes = 12;
	std::vector<std::byte> return_val(audio_data.size() + crypto_secretbox_MACBYTES + header_size_bytes);

	std::copy(header.begin(), header.end(), return_val.begin());
	
	std::array<std::byte, crypto_secretbox_NONCEBYTES> nonce{{}};
	std::ranges::fill(nonce, std::byte(0));
	
	std::ranges::copy(header, nonce.begin());
	
	(void)sodium_init();
	
	crypto_secretbox_easy(
		(unsigned char*)return_val.data() + header_size_bytes,
		(unsigned char*)audio_data.data(),
		audio_data.size(),
		(unsigned char*)nonce.data(),
		(unsigned char*)m_secret_key.data()
	);
	return return_val;
}

cerwy::eager_task<void> discord_voice_connection_impl::send_heartbeat() {
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
	while (is_alive) {
		boost::asio::steady_timer timer(context());
		timer.expires_after(std::chrono::milliseconds(heartbeat_interval));
		auto ec = co_await timer.async_wait(use_task_return_ec);
		if (ec) {
			break;
		}
		std::string hb = fmt::format(R"(
			{{
			"op":3,
			"d":{}
			}}
			)", m_hb_number++);
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
	//TODO: add comment on why this is * 3/4
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
}

void discord_voice_connection_impl::on_ready(nlohmann::json data) {
	ssrc = data["ssrc"].get<uint32_t>();
	m_ip = data["ip"].get<std::string>();
	m_port = data["port"].get<int>();
	m_modes = data["modes"].get<std::vector<std::string>>();	
	connect_udp();	
}

void discord_voice_connection_impl::send_op1_select_protocol() const {
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
}})",m_my_endpoint.address().to_string(), m_my_endpoint.port());
	
	//std::cout << msg << std::endl;
	
	socket.send_thing(std::move(msg));
}

void discord_voice_connection_impl::on_session_discription(nlohmann::json data) {
	m_secret_key = data["secret_key"].get<std::vector<std::byte>>();
	std::cout << data["mode"] << std::endl;
	waiter->set_value();//finish setup
}

cerwy::eager_task<void> discord_voice_connection_impl::connect_udp() {
	//std::cout << m_ip << ' ' << m_port << std::endl;

	
	
	auto resolver = boost::asio::ip::udp::resolver(context());
	const auto port = std::to_string(m_port);
	auto [ec,results] = co_await resolver.async_resolve(m_ip, port, use_task_return_tuple2);
	
	if (ec) {
		//die? never happens?
		int u = 0;
		std::cout << "failed 'connecting' udp 1" << ec << std::endl;
	}

	auto [ec2,ep] = co_await boost::asio::async_connect(voice_socket, results, use_task_return_tuple2);
	if (ec2) {
		int y = 0;
		std::cout << "failed connecting udp 2" << ec << std::endl;
	} else {
		
	}


	co_await do_ip_discovery();
	send_op1_select_protocol();
}

cerwy::eager_task<void> discord_voice_connection_impl::do_ip_discovery() {
	std::array<std::byte, 74> data_to_send{{}};
	//(uint16_t&)data_to_send[0] = htons(0x1);
	(uint8_t&)data_to_send[1] = 0x1;
	(uint16_t&)data_to_send[2] = htons(70);
	(uint32_t&)data_to_send[4] = htonl(ssrc);	
	//data_to_send[8];
	//std::copy(m_ip.begin(), m_ip.end(), (char*)&data_to_send[8]);
	(uint16_t&)data_to_send[72] = htons(m_port);

	auto [ec, n] = co_await voice_socket.async_send(boost::asio::buffer(data_to_send), use_task_return_tuple2);

	std::array<std::byte, 74> msg{{}};

	//boost::asio::ip::udp::endpoint server_ep;
	
	auto [ec2,n2] = co_await voice_socket.async_receive(boost::asio::buffer(msg),use_task_return_tuple2);

	//std::cout << server_ep.address().to_string() << ' ' << server_ep.port() << std::endl;
	
	m_my_endpoint = boost::asio::ip::udp::endpoint(
		boost::asio::ip::make_address((char*)&msg[8]),
		ntohs((uint16_t&)msg[72])
	);
	
	co_return;
}

cerwy::eager_task<boost::system::error_code> discord_voice_connection_impl::send_voice_data_udp(std::span<const std::byte> data) {
	auto [ec, n] = co_await voice_socket.async_send(boost::asio::buffer(data.data(),data.size_bytes()), use_task_return_tuple2);
	co_return ec;
}

cerwy::eager_task<boost::system::error_code> discord_voice_connection_impl::wait(std::chrono::milliseconds time) {
	boost::asio::steady_timer timer(context());
	timer.expires_after(time);
	auto ec3 = co_await timer.async_wait(use_task_return_ec);
	co_return ec3;
}
