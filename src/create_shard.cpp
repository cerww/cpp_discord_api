#include "create_shard.h"
#include "task_completion_handler.h"
#include "web_socket_helpers.h"
#include "client.h"
#include "rename_later_5.h"
#include <fmt/core.h>

namespace discord_ec{
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

cerwy::task<void> create_shard(const int shard_number, client* t_parent, boost::asio::io_context& ioc, std::string_view gateway) {
	try {

		boost::asio::ssl::context ssl_ctx{ boost::asio::ssl::context_base::tlsv12 };
		boost::asio::ip::tcp::resolver resolver(ioc);

		boost::beast::websocket::stream<ssl_stream<boost::asio::ip::tcp::socket>> socket = co_await wss_from_uri(gateway, resolver, ssl_ctx);
		rename_later_5 rename_me(socket);
		
		boost::beast::multi_buffer buffer = {};

		shard me(shard_number, &rename_me, t_parent, ioc);
		co_await me.connect_http_connection();
		t_parent->add_shard(&me);

		while (true) {
			std::cerr << "recieving stuffs\n";
			auto[ec, n] = co_await socket.async_read(buffer, use_task_return_tuple2);
			
			if (ec) {
				if(ec.value() == discord_ec::unknown_error) {
					socket = co_await wss_from_uri(gateway, resolver, ssl_ctx);
					rename_me.maybe_restart();
					me.on_reconnect();
				}else if(ec.value() == discord_ec::not_authenticated) {
					socket = co_await wss_from_uri(gateway, resolver, ssl_ctx);
					rename_me.maybe_restart();
					me.on_reconnect();
				}else if(ec.value() == discord_ec::authentication_failed) {					
					fmt::print("authentication failed");
					break;
				}else if(ec.value() == discord_ec::already_authenticated) {
					fmt::print("already authenticated");
					break;
				}else if(ec.value() == discord_ec::invalid_seq) {
					socket = co_await wss_from_uri(gateway, resolver, ssl_ctx);
					rename_me.maybe_restart();
					me.on_reconnect();
				}else if(ec.value() == discord_ec::rate_limited) {
					fmt::print("rate limited");
					break;
				}else if (ec.value() == discord_ec::timeout) {
					fmt::print("session timedout, reconnecting");
					socket = co_await wss_from_uri(gateway, resolver, ssl_ctx);
					rename_me.maybe_restart();
					me.on_reconnect();
				}else if (ec.value() == discord_ec::sharding_required) {
					fmt::print("sharding required");
					break;
				}
			}
			auto data = buffer.data();
			auto json = nlohmann::json::parse(
				data |
				ranges::view::transform([](auto a) {return std::string_view((const char*)a.data(), a.size()); }) |
				ranges::view::join |
				ranges::to_<std::string>()
			);
			buffer.consume(n);
			auto op = json["op"].get<int>();
			me.doStuff(std::move(json), op);			
		}
	} catch (...) {
		std::cerr << "failed for some reason\n";
	}
}
