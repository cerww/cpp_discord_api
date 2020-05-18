#include "init_shard.h"
#include "task_completion_handler.h"
#include "web_socket_helpers.h"
#include "client.h"
#include "rename_later_5.h"
#include <fmt/core.h>
#include "executor_binder.h"

namespace discord_ec {
	constexpr int unknown_error = 4000;
	constexpr int unknown_opcode = 4001;
	constexpr int decode_error = 4002;
	constexpr int not_authenticated = 4003;
	constexpr int authentication_failed = 4004;
	constexpr int already_authenticated = 4005;
	constexpr int invalid_seq = 4007;
	constexpr int rate_limited = 4008;
	constexpr int timeout = 4009;
	constexpr int invalid_shard = 4010;
	constexpr int sharding_required = 4011;
}

cerwy::task<void> init_shard(const int shard_number, shard& me, boost::asio::io_context& ioc, std::string_view gateway) {
	try {
		co_await reconnect_wss_from_url(me.m_socket, gateway, me.resolver(), me.ssl_context());

		auto& m_socket = me.m_socket;

		me.m_client = std::make_unique<rename_later_5>(m_socket);
		rename_later_5& m_client = *me.m_client;

		co_await me.connect_http_connection();

		boost::beast::multi_buffer buffer = {};

		auto& strand = me.strand();

		while (true) {
			std::cerr << "recieving stuffs\n";
			const auto [ec, n] = co_await m_socket.async_read(buffer, cerwy::bind_executor(strand, use_task_return_tuple2));

			if (ec) {
				std::cout << ec << std::endl;
				if (ec.value() == discord_ec::unknown_error) {
					co_await reconnect_wss_from_url(m_socket, gateway, me.resolver(), me.ssl_context());
					m_client.maybe_restart();
					me.on_reconnect();
				}
				else if (ec.value() == discord_ec::not_authenticated) {
					co_await reconnect_wss_from_url(m_socket, gateway, me.resolver(), me.ssl_context());
					m_client.maybe_restart();
					me.on_reconnect();
				}
				else if (ec.value() == discord_ec::authentication_failed) {
					fmt::print("authentication failed");
					break;
				}
				else if (ec.value() == discord_ec::already_authenticated) {
					fmt::print("already authenticated");
					break;
				}
				else if (ec.value() == discord_ec::invalid_seq) {
					co_await reconnect_wss_from_url(m_socket, gateway, me.resolver(), me.ssl_context());
					m_client.maybe_restart();
					me.on_reconnect();
				}
				else if (ec.value() == discord_ec::rate_limited) {
					fmt::print("rate limited");
					break;
				}
				else if (ec.value() == discord_ec::timeout) {
					fmt::print("session timedout, reconnecting");
					co_await reconnect_wss_from_url(m_socket, gateway, me.resolver(), me.ssl_context());
					m_client.maybe_restart();
					me.on_reconnect();
				}
				else if (ec.value() == discord_ec::sharding_required) {
					fmt::print("sharding required");
					break;
				}
				else {
					std::cout << ec << std::endl;
					m_client.close((int)boost::beast::websocket::close_code::normal);
					co_return;
				}
			}
			auto data = buffer.data();
			auto json = nlohmann::json::parse(
				data |
				ranges::views::transform([](auto a) { return std::string_view((const char*)a.data(), a.size()); }) |
				ranges::views::join |
				ranges::to<std::string>()
			);
			buffer.consume(n);
			auto op = json["op"].get<int>();
			me.doStuff(std::move(json), op);
		}
	} catch (...) {
		std::cerr << "failed for some reason\n";
	}
}
