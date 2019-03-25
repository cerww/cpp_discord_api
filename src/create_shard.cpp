#include "create_shard.h"
#include "task_completion_handler.h"
#include "web_socket_helpers.h"
#include "client.h"
#include "rename_later_5.h"
#include <fmt/core.h>

cerwy::task<void> create_shard(int shardN, client* t_parent, boost::asio::io_context& ioc, std::string_view gateway) {
	try {

		boost::asio::ssl::context ssl_ctx{ boost::asio::ssl::context_base::tlsv12 };
		boost::asio::ip::tcp::resolver resolver(ioc);

		boost::beast::websocket::stream<ssl_stream<boost::asio::ip::tcp::socket>> socket = co_await wss_from_uri(gateway, resolver, ssl_ctx);
		rename_later_5 rename_me(socket);
		
		boost::beast::multi_buffer buffer = {};

		shard me(shardN, &rename_me, t_parent, ioc);
		co_await me.connect_http_connection();
		t_parent->add_shard(&me);

		while (true) {
			std::cerr << "recieving stuffs\n";
			auto[ec, n] = co_await socket.async_read(buffer, use_task_return_tuple2);
			if (ec) {
				if(ec.value() == 4003) {
					socket = co_await wss_from_uri(gateway, resolver, ssl_ctx);
					rename_me.maybe_restart();
					me.on_reconnect();
				}else if(ec.value() == 4004) {
					fmt::print("authentication failed");
					break;
				}else if(ec.value() == 4005) {
					fmt::print("already authenticated");
					break;
				}else if(ec.value() == 4007) {
					socket = co_await wss_from_uri(gateway, resolver, ssl_ctx);
					rename_me.maybe_restart();
					me.on_reconnect();
				}else if(ec.value() == 4008) {
					fmt::print("rate limited");
					break;
				}else if (ec.value() == 4009) {
					fmt::print("session timedout, reconnecting");
					socket = co_await wss_from_uri(gateway, resolver, ssl_ctx);
					rename_me.maybe_restart();
					me.on_reconnect();
				}else if (ec.value() == 4011) {
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
