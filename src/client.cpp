#include "client.h"
#include "internal_shard.h"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>


client::client(int threads, intents intents):
	m_ioc(std::in_place_type<boost::asio::io_context>,threads),
	m_threads(std::max(threads-1,0)),
	m_intents(intents){
	
}

client::client(boost::asio::io_context& ioc, intents intents):
m_ioc(std::in_place_type<boost::asio::io_context*>,&ioc),
m_intents(intents){
	
}

void client::run() {

	do_gateway_stuff();
	//m_th

	boost::asio::executor_work_guard work_guard(context().get_executor());
	ranges::generate(m_threads, [&]() {
		return std::thread([&]() {
			context().run();
		});
	});


	context().run();
	int u = 0;
}

void client::stop() {
	context().stop();
}

void client:: set_token(std::string token, const token_type type) {
	m_token = token;
	switch (type) {
	case token_type::BOT: m_authToken = "Bot " + std::move(token);
		break;
	case token_type::BEARER: m_authToken = "Bearer " + std::move(token);
		break;
	default: break;
	}
}


// void client::set_up_request(boost::beast::http::request<boost::beast::http::string_body>& req) const {
// 	req.set("Application", "cerwy");
// 	req.set(boost::beast::http::field::authorization, m_authToken);
// 	req.set(boost::beast::http::field::host, "discord.com"s);
// 	req.set(boost::beast::http::field::user_agent, "watland");
// 	req.set(boost::beast::http::field::accept, "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
// 	req.set(boost::beast::http::field::accept_language, "en-US,en;q=0.5");
// 	req.keep_alive(true);
// }


void client::rate_limit_global(const std::chrono::system_clock::time_point tp) {
	std::unique_lock<std::mutex> locky(m_global_rate_limit_mut,std::try_to_lock);
	//only 1 shard needs to call this to rate limit every shard
	if(!locky) {
		return;
	}
	const auto now = std::chrono::system_clock::now();
	if (m_last_global_rate_limit - now < std::chrono::seconds(3)){//so i don't rate_limit myself twice
		m_last_global_rate_limit = now;
		for (auto& shard : m_shards) {
			shard->rate_limit(tp);
		}
	}	
}

void client::do_gateway_stuff() {
	m_getGateway();
	for (int i = 0; i < m_num_shards; ++i) {
		m_shards.emplace_back(std::make_unique<internal_shard>(i, this, context(), m_gateway, m_intents));
	}
}

void client::m_getGateway() {
	boost::asio::io_context ioc;//m_ioc.run is called after this finishes, so need a different ioc
	boost::asio::ip::tcp::resolver resolver{ ioc };

	boost::asio::ssl::context ssl_context{ boost::asio::ssl::context::tlsv12_client };
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket{ ioc, ssl_context };
	boost::beast::flat_buffer buffer;

	boost::asio::connect(ssl_socket.next_layer(), resolver.resolve("discord.com", "https"));
	ssl_socket.handshake(boost::asio::ssl::stream_base::client);
	boost::beast::http::request<boost::beast::http::string_body> request(boost::beast::http::verb::get, "/api/v6/gateway/bot", 11);
	request.set("Application", "cerwy");
	request.set(boost::beast::http::field::authorization, m_authToken);
	request.set("Host", "discord.com"s);
	request.set("Upgrade-Insecure-Requests", "1");
	request.set(boost::beast::http::field::user_agent, "potato_world");
		
	request.keep_alive(true);

	request.set(boost::beast::http::field::accept, "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	//request.set(boost::beast::http::field::accept_encoding, "gzip,deflate,br");

	request.set(boost::beast::http::field::accept_language, "en-US,en;q=0.5");

	request.prepare_payload();
	boost::beast::http::write(ssl_socket, request);
	boost::beast::http::response<boost::beast::http::string_body> response;

	boost::beast::http::read(ssl_socket, buffer, response);
	if(response.result_int() == 401) {
		throw std::runtime_error("unauthorized. bad token?");
	}
	nlohmann::json yay = nlohmann::json::parse(response.body());
	
	m_gateway = yay["url"].get<std::string>() +"/?v=6&encoding=json";
	if (m_num_shards == 0) {
		m_num_shards = yay["shards"].get<int>();
	}
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