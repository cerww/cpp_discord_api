#include "discord_http_connection.h"
#include "client.h"
#include <queue>

size_t major_param_id(const std::string& s) {
	return 0;
}

discord_http_connection::discord_http_connection(client* t):m_client(t) {
	const auto results = resolver.resolve("discordapp.com", "https");
	boost::asio::connect(m_ssl_stream.next_layer(), results.begin(), results.end());	
	m_ssl_stream.handshake(boost::asio::ssl::stream_base::client);
	m_thread = std::thread([&](){
		int things_sent = 0;
		std::unordered_map<size_t, std::chrono::steady_clock::time_point> reset_times;
		
		while (!m_done.load()) {
			if (m_global_rate_limited.load()) {
				std::this_thread::sleep_until(m_tp);
				m_global_rate_limited.store(false);
			}
			//if (discord_request r; abcd.try_pop(r)) {
				//std::chrono::milliseconds rate_limit_reset;
			discord_request r = abcd.pop();
			std::cout << r.req << std::endl;

			if (r.state->ref_count == 0)continue;
			{//else
				std::lock_guard<std::mutex> locky(r.state->mut);
				boost::beast::error_code ec;
				boost::beast::http::write(m_ssl_stream, r.req,ec);
				boost::beast::http::read(m_ssl_stream, m_buffer, r.state->res,ec);
				while (r.state->res.result_int() == 429) {
					try {							
						if (r.state->res.at("X-RateLimit-Global") == "true") {
							m_client->rated_limit_global(std::chrono::steady_clock::now() + std::chrono::milliseconds(std::stoi(r.state->res.at("Retry-After").to_string())));
						}
					} catch (...) {}
					std::this_thread::sleep_for(std::chrono::milliseconds(std::stoi(r.state->res.at("Retry-After").to_string())));
					boost::beast::http::write(m_ssl_stream, r.req);
					boost::beast::http::read(m_ssl_stream, m_buffer, r.state->res);
				}try {
					if(r.state->res.at("X-RateLimit-Remaining") == "0") {
						std::this_thread::sleep_until(std::chrono::steady_clock::time_point(std::chrono::seconds(std::stoi(r.state->res.at("X-RateLimit-Reset").to_string()))));
					}
				} catch (...) {}
				std::cout << r.state->res << std::endl;
				r.state->done = true;
				++things_sent;				
			}
			r.state->cv.notify_all();
			//}
			//else {std::this_thread::sleep_for(10ms);	}
		}
	});
}

template<typename Socket,typename Buffer>
void send_thingy(discord_request& r, Socket& sock, Buffer& buffer, client* parent) {
	std::cout << r.req << std::endl;
	if (r.state->ref_count == 1)return;
	
	std::lock_guard<std::mutex> locky(r.state->mut);
	boost::beast::error_code ec;
	boost::beast::http::write(sock, r.req, ec);
	boost::beast::http::read(sock, buffer, r.state->res, ec);
	while (r.state->res.result_int() == 429) {
		try {
			if (r.state->res.at("X-RateLimit-Global") == "true") 
				parent->rated_limit_global(std::chrono::steady_clock::now() + std::chrono::milliseconds(std::stoi(r.state->res.at("Retry-After").to_string())));			
		}catch (...) {}
		std::this_thread::sleep_for(std::chrono::milliseconds(std::stoi(r.state->res.at("Retry-After").to_string())));
		boost::beast::http::write(sock, r.req);
		boost::beast::http::read(sock, buffer, r.state->res);
	}try {
		if (r.state->res.at("X-RateLimit-Remaining") == "0") {
			std::this_thread::sleep_until(std::chrono::steady_clock::time_point(std::chrono::seconds(std::stoi(r.state->res.at("X-RateLimit-Reset").to_string()))));
		}
	}
	catch (...) {}
	std::cout << r.state->res << std::endl;
	r.state->done = true;
}

std::future<void> http_conn(d::subscriber_thingy_async<std::variant<discord_request, std::chrono::milliseconds>>& hand, client* parent){
	boost::asio::io_context m_ioc;
	boost::asio::ip::tcp::resolver resolver{ m_ioc };

	boost::asio::ssl::context m_sslCtx{ boost::asio::ssl::context::tlsv12_client };
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_ssl_stream{ m_ioc, m_sslCtx };
	boost::beast::flat_buffer m_buffer;

	const auto results = resolver.resolve("discordapp.com", "https");
	boost::asio::connect(m_ssl_stream.next_layer(), results.begin(), results.end());
	m_ssl_stream.handshake(boost::asio::ssl::stream_base::client);

	for co_await(auto&& rq: hand) {
		std::visit([&](auto&& thing) {
			using type = std::decay_t<decltype(thing)>;
			if constexpr(std::is_same_v<type, discord_request>) {				
				send_thingy(thing, m_ssl_stream, m_buffer, parent);
				thing.state->cv.notify_all();
			}
			else if constexpr(std::is_same_v<type, std::chrono::milliseconds>) {//
				std::this_thread::sleep_for(thing);
			}

		}, rq);
	}
}
