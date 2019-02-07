#include "client.h"
#include "shard.h"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
//#include "randomThings.h"

client::client() {
	m_ws_hub.onMessage([&](uWS::WebSocket<uWS::CLIENT>* clientu, char* msg_t, size_t size, uWS::OpCode op) {
		if(op == uWS::OpCode::TEXT)
			m_shards[clientu]->add_event(std::string(msg_t,msg_t + size));
		else if(op == uWS::OpCode::CLOSE) {
			
		}
	});

	m_ws_hub.onDisconnection([&](uWS::WebSocket<uWS::CLIENT>* clientu,int code,char* msg,size_t len){
		if(code == 4000) {
			m_shards[clientu]->reconnect();
		}if(code == 4003) {
			std::cout << "not authenticated" << std::endl;
		}
		if(code == 4004) {
			std::cout << "authentication failed" << std::endl;
		}
		if(code == 4005) {
			std::cout << "already authenticated" << std::endl;
		}
		if(code == 4007) {
			m_shards[clientu]->reconnect();
		}
		if(code == 4008) {
			std::cout << "rate limited" << std::endl;
		}
		if(code == 4009) {
			std::cout << "session timeout" << std::endl;
			m_shards[clientu]->reconnect();
		}
		if(code == 4011) {
			std::cout << "sharding required" << std::endl;
		}
	});

	m_ws_hub.onConnection([&](uWS::WebSocket<uWS::CLIENT>* web_socket, uWS::HttpRequest){
		const auto it = std::find_if(m_shards.begin(),m_shards.end(),[](auto& val){
			return val.second->is_disconnected();
		});//the socket each shard is associated with is determined after send_resume();
		if(it!=m_shards.end()) {
			auto temp = m_shards.extract(it);
			temp.key() = web_socket;
			shard& shardy = *m_shards.insert(std::move(temp)).node.mapped();//wat
			shardy.send_resume();
		}else{
			m_shards.insert(std::make_pair(web_socket,std::make_unique<shard>((int)m_shards.size(), web_socket, this)));
		}
	});

	m_ws_hub.onError([&](void* user){
		std::cout << "couldn't connect try again" << std::endl;
	});
}

void client::run() {
	m_getGateway();
	for (int i = 0; i < m_num_shards; ++i)
		m_ws_hub.connect(m_gateway);

	m_ws_hub.run();
}

void client::setToken(const tokenType type, std::string token) {
	m_token = token;
	switch (type) {
	case tokenType::BOT: m_authToken = "Bot " + std::move(token);
		break;
	case tokenType::BEARER: m_authToken = "Bearer " + std::move(token);
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
	//only 1 shard needs to rate limit every shard
	if(!locky) {
		return;
	}
	const auto now = std::chrono::system_clock::now();
	if (m_last_global_rate_limit - now < std::chrono::seconds(5)){//so i don't rate_limit myself twice
		m_last_global_rate_limit = now;
		for(auto& i:m_shards) {
			i.second->rate_limit(tp);
		}
	}	
}

void client::m_getGateway() {
	boost::asio::io_context ioc;
	boost::asio::ip::tcp::resolver resolver{ ioc };

	boost::asio::ssl::context m_sslCtx{ boost::asio::ssl::context::tlsv12_client };
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_ssl_stream{ ioc, m_sslCtx };
	boost::beast::flat_buffer m_buffer;

	const auto results = resolver.resolve("discordapp.com", "https");
	boost::asio::connect(m_ssl_stream.next_layer(), results.begin(), results.end());
	m_ssl_stream.handshake(boost::asio::ssl::stream_base::client);
	boost::beast::http::request<boost::beast::http::string_body> req(boost::beast::http::verb::get, "/api/v6/gateway/bot", 11);
	req.set("Application", "cerwy");
	req.set(boost::beast::http::field::authorization, m_token);
	req.set("Host", "discordapp.com"s);
	req.set("Upgrade-Insecure-Requests", "1");
	req.set(boost::beast::http::field::user_agent, "watland");
		
	req.keep_alive(true);

	req.set(boost::beast::http::field::accept, "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	//req.set(boost::beast::http::field::accept_encoding, "gzip,deflate,br");

	req.set(boost::beast::http::field::accept_language, "en-US,en;q=0.5");

	req.prepare_payload();
	boost::beast::http::write(m_ssl_stream, req);
	boost::beast::http::response<boost::beast::http::string_body> response;

	boost::beast::http::read(m_ssl_stream, m_buffer, response);

	nlohmann::json yay = nlohmann::json::parse(response.body());

	m_gateway = yay["url"].get<std::string>() +"/?v=6&encoding=json";
	m_num_shards = yay["shards"].get<int>();
	//m_num_shards = 2;
	std::cout << m_gateway << std::endl;
}

nlohmann::json client::presence()const {
	nlohmann::json retVal;
	retVal["since"];
	retVal["game"]["name"] = gameName;
	retVal["game"]["type"] = 0;
	retVal["status"] = enum_to_string(status);
	retVal["afk"] = status == Status::idle;

	return retVal;
}