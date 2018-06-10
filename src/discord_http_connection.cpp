#include "client.h"
#include "discord_http_connection.h"
#include <queue>

size_t get_major_param_id(std::string_view s) {
	s.remove_prefix(7);//sizeof("/api/v6") - 1;
	const auto start = s.find_first_of("1234567890");
	if (start == std::string_view::npos) 
		return 0;
	s.remove_prefix(start);
	const size_t end = s.find('/');
	return std::stoull(std::string(s.begin(), s.begin() + (end != std::string_view::npos ? end : s.size())));	
}

discord_http_connection::discord_http_connection(client* t):m_client(t) {
	const auto results = resolver.resolve("discordapp.com", "https");
	boost::asio::connect(m_ssl_stream.next_layer(), results.begin(), results.end());	
	m_ssl_stream.handshake(boost::asio::ssl::stream_base::client);
	m_thread = std::thread([this](){
		while (!m_done.load()) {			
			if(m_rate_limited.empty())
				send_to_discord(m_request_queue.pop());
			else{				
				if(std::optional<discord_request> r = m_request_queue.try_pop_until(std::get<1>(m_rate_limited[0])); r) {
					send_to_discord(std::move(r.value()));
				}else {
					auto thing = std::move(m_rate_limited[0]);
					m_rate_limited.erase(m_rate_limited.begin());
					for(auto& request:std::get<2>(thing))
						send_to_discord(std::move(request));					
				}
			}
		}
	});
}

void discord_http_connection::send_to_discord(discord_request r) {
	std::cout << r.req << std::endl;	
	const size_t major_param_id = get_major_param_id(std::string_view(r.req.target().data(), r.req.target().size()));
	const auto it = std::find_if(m_rate_limited.begin(), m_rate_limited.end(), [&](const auto& thing){
		return std::get<0>(thing) == major_param_id;
	});
	if(it != m_rate_limited.end()) {
		std::get<2>(*it).push_back(std::move(r));
		return;
	}
	if (m_global_rate_limited.load()) {
		std::this_thread::sleep_until(m_rate_limted_until);
		m_global_rate_limited.store(false);
	}
	send_to_discord_(r,major_param_id);
	r.state->cv.notify_all();
}

void discord_http_connection::send_to_discord_(discord_request& r,size_t major_param_id_) {	
	std::lock_guard<std::mutex> locky(r.state->mut);
	boost::beast::error_code ec;
	boost::beast::http::write(m_ssl_stream, r.req, ec);
	boost::beast::http::read(m_ssl_stream, m_buffer, r.state->res, ec);
	if (ec) std::cout << "rawrrrrr " << ec << std::endl;
	while (r.state->res.result_int() == 429) {
		try {
			if (r.state->res.at("X-RateLimit-Global") == "true")
				m_client->rated_limit_global(std::chrono::steady_clock::now() + std::chrono::milliseconds(std::stoi(r.state->res.at("Retry-After").to_string())));
		}catch (...) {}
		r.state->res.clear();
		r.state->res.body().clear();
		std::this_thread::sleep_for(std::chrono::milliseconds(std::stoi(r.state->res.at("Retry-After").to_string())));
		boost::beast::http::write(m_ssl_stream, r.req, ec);
		boost::beast::http::read(m_ssl_stream, m_buffer, r.state->res, ec);
		if (ec)std::cout << "rawrrrrr " << ec << std::endl;
	}try {
		if (r.state->res.at("X-RateLimit-Remaining") == "0") {			
			const auto time = std::chrono::system_clock::time_point(std::chrono::seconds(std::stoi(r.state->res.at("X-RateLimit-Reset").to_string())));
			m_rate_limited.insert(std::upper_bound(m_rate_limited.begin(), m_rate_limited.end(), time, [](const std::chrono::system_clock::time_point& a, const auto& b) {
				return a < std::get<1>(b);
			}), { major_param_id_,time,{} });
		}
	}catch (...) {}
	std::cout << r.state->res << std::endl;
	r.state->done = true;
}


/*
template<typename Socket,typename Buffer>
void send_thingy(discord_request& r, Socket& sock, Buffer& buffer, client* parent) {
	std::cout << r.req << std::endl;
	
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
*/