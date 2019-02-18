#include "create_shard.h"
#include "task_completion_handler.h"
#include "web_socket_helpers.h"
#include "client.h"
#include "rename_later_5.h"

cerwy::task<void> create_shard(int shardN, client* t_parent, boost::asio::io_context& ioc, std::string_view gateway) {
	try {

		boost::asio::ssl::context ssl_ctx{ boost::asio::ssl::context_base::tlsv12 };
		boost::asio::ip::tcp::resolver resolver(ioc);

		boost::beast::websocket::stream<ssl_stream<boost::asio::ip::tcp::socket>> socket = co_await wss_from_uri(gateway, resolver, ssl_ctx);

		boost::beast::multi_buffer buffer = {};
		rename_later_5 rename_me(socket);

		shard me(shardN, &rename_me, t_parent, ioc);
		t_parent->add_shard(&me);

		while (true) {
			auto[ec, n] = co_await socket.async_read(buffer, use_task_return_tuple2);
			if (ec) {
				if(ec.value() == 4003) {
					socket = co_await wss_from_uri(gateway, resolver, ssl_ctx);
				}else if(ec.value() == 4004) {
					std::cerr << "authentication failed\n";
					break;
				}else if(ec.value() == 4005) {
					std::cerr << "already authenticated\n";
					break;
				}else if(ec.value() == 4007) {
					socket = co_await wss_from_uri(gateway, resolver, ssl_ctx);
				}else if(ec.value() == 4008) {
					std::cerr << "rate limited\n";
					break;
				}else if (ec.value() == 4009) {
					std::cerr << "session timeout" << std::endl;
					socket = co_await wss_from_uri(gateway, resolver, ssl_ctx);
				}else if (ec.value() == 4011) {
					std::cerr << "sharding required" << std::endl;
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
	co_return;
}
