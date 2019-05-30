#include "client.h"
#include "shard.h"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
//#include "randomThings.h"
#include "init_shard.h"

client::client(int threads):
	m_ioc(threads),
	m_threads(std::max(threads-1,0)){
	
}

void client::run() {
	m_getGateway();
	for (int i = 0; i < m_num_shards; ++i) {
		//create_shard(i, this, m_ioc, m_gateway);
		m_shards.emplace_back(std::make_unique<shard>(i,this,m_ioc,m_gateway));
	}
	//m_th

	boost::asio::executor_work_guard work_guard(m_ioc.get_executor());
	ranges::generate(m_threads, [&]() {
		return std::thread([&]() {
			m_ioc.run();
		});
	});


	m_ioc.run();
	int u = 0;
}

void client::stop() {
	m_ioc.stop();
}

void client::setToken(const token_type type, std::string token) {
	m_token = token;
	switch (type) {
	case token_type::BOT: m_authToken = "Bot " + std::move(token);
		break;
	case token_type::BEARER: m_authToken = "Bearer " + std::move(token);
		break;
	default: break;
	}
}

void client::set_up_request(boost::beast::http::request<boost::beast::http::string_body>& req) const {
	req.set("Application", "cerwy");
	req.set(boost::beast::http::field::authorization, m_authToken);
	req.set(boost::beast::http::field::host, "discordapp.com"s);
	req.set(boost::beast::http::field::user_agent, "watland");
	req.set(boost::beast::http::field::accept, "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	req.set(boost::beast::http::field::accept_language, "en-US,en;q=0.5");
	req.keep_alive(true);
}

void client::rate_limit_global(const std::chrono::system_clock::time_point tp) {
	std::unique_lock<std::mutex> locky(m_global_rate_limit_mut,std::try_to_lock);
	//only 1 shard needs to call this to rate limit every shard
	if(!locky) {
		return;
	}
	const auto now = std::chrono::system_clock::now();
	if (m_last_global_rate_limit - now < std::chrono::seconds(3)){//so i don't rate_limit myself twice
		m_last_global_rate_limit = now;
		for(auto& i:m_shards) {
			i->rate_limit(tp);
		}
	}	
}

void client::m_getGateway() {
	boost::asio::io_context ioc;//m_ioc.run is called after this finishes, so need a different ioc
	boost::asio::ip::tcp::resolver resolver{ ioc };

	boost::asio::ssl::context ssl_context{ boost::asio::ssl::context::tlsv12_client };
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket{ ioc, ssl_context };
	boost::beast::flat_buffer buffer;

	const auto results = resolver.resolve("discordapp.com", "https");
	boost::asio::connect(ssl_socket.next_layer(), results.begin(), results.end());
	ssl_socket.handshake(boost::asio::ssl::stream_base::client);
	boost::beast::http::request<boost::beast::http::string_body> request(boost::beast::http::verb::get, "/api/v6/gateway/bot", 11);
	request.set("Application", "cerwy");
	request.set(boost::beast::http::field::authorization, m_token);
	request.set("Host", "discordapp.com"s);
	request.set("Upgrade-Insecure-Requests", "1");
	request.set(boost::beast::http::field::user_agent, "potato_world");
		
	request.keep_alive(true);

	request.set(boost::beast::http::field::accept, "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	//req.set(boost::beast::http::field::accept_encoding, "gzip,deflate,br");

	request.set(boost::beast::http::field::accept_language, "en-US,en;q=0.5");

	request.prepare_payload();
	boost::beast::http::write(ssl_socket, request);
	boost::beast::http::response<boost::beast::http::string_body> response;

	boost::beast::http::read(ssl_socket, buffer, response);

	nlohmann::json yay = nlohmann::json::parse(response.body());

	m_gateway = yay["url"].get<std::string>() +"/?v=6&encoding=json";
	m_num_shards = yay["shards"].get<int>();
	//m_num_shards = 2;
	//std::cout << m_gateway << std::endl;
	fmt::print(m_gateway);
	fmt::print("\n");	
}

/*
nlohmann::json client::presence()const {
	nlohmann::json retVal;
	retVal["since"];
	retVal["game"]["name"] = gameName;
	retVal["game"]["type"] = 0;
	retVal["status"] = enum_to_string(status);
	retVal["afk"] = status == Status::idle;

	return retVal;
}
*/