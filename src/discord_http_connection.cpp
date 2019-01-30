#include "client.h"
#include "discord_http_connection.h"
#include <charconv>
#include <range/v3/core.hpp>

size_t get_major_param_id(std::string_view s) {
	s.remove_prefix(7);//sizeof("/api/v6") - 1;
	const auto start = s.find_first_of("1234567890");
	if (start == std::string_view::npos) 
		return 0;
	s.remove_prefix(start);
	const auto end = std::find(s.begin(), s.end(), '/');
	const auto length = std::distance(s.begin(), end);
	size_t ret = 0;
	std::from_chars(s.data(), s.data() + length,ret);
	return ret;
}

discord_http_connection::discord_http_connection(client* t):m_client(t) {
	connect();
	m_thread = std::thread([this](){
		while (!m_done.load()) {
			if(m_rate_limited_requests.empty())
				send_to_discord(m_request_queue.pop());
			else if(std::optional<discord_request> r = m_request_queue.try_pop_until(std::get<1>(m_rate_limited_requests[0])); r) {
				send_to_discord(std::move(r.value()));
			}else{//std::chrono::system_clock::now() >= std::get<1>(m_rate_limited_requests[0])
				auto requests_to_send = std::move(m_rate_limited_requests[0]);
				m_rate_limited_requests.erase(m_rate_limited_requests.begin());
				for(auto& request:std::get<2>(requests_to_send))
					send_to_discord(std::move(request));					
			}
		}		
	});
	
}

void discord_http_connection::send_to_discord(discord_request r) {	
	const size_t major_param_id = get_major_param_id(std::string_view(r.req.target().data(), r.req.target().size()));
	const auto it = std::find_if(m_rate_limited_requests.begin(), m_rate_limited_requests.end(), [&](const auto& thing){
		return std::get<0>(thing) == major_param_id;
	});
	if(it != m_rate_limited_requests.end()) {
		std::get<2>(*it).push_back(std::move(r));
		return;
	}
	if (m_global_rate_limited.load()) {
		std::this_thread::sleep_until(m_rate_limted_until);
		m_global_rate_limited.store(false);
	}
	if(send_to_discord_(r,major_param_id))
		r.state->cv.notify_all();
}

template<int i>
struct get_n{
	template<typename T>
	constexpr decltype(auto) operator()(T&& t) const{
		return std::get<i>(std::forward<T>(t));
	}
};

//returns wether or not it's not local rate limited
bool discord_http_connection::send_to_discord_(discord_request& r,size_t major_param_id_) {
	std::lock_guard<std::mutex> locky(r.state->mut);
	boost::beast::error_code ec; 
	send_rq(r);
	//this is "while" not "if" because of global rate limits
	//it shuld keep trying to send the same request if it's global rate limited
	//cuz if i doin't do this, the request will go into the queue, then it'll sleep, then it'll prolyl send the request(it might sned a different request),
	//i can skip adding the request into the queue by doing this
	while (r.state->res.result_int() == 429) {
		std::cout << "rate limited" << '\n';
		nlohmann::json json_body = nlohmann::json::parse(r.state->res.body());
		const auto tp = std::chrono::system_clock::now() + std::chrono::milliseconds(json_body["retry_after"].get<size_t>());
		if(json_body["global"].get<bool>()) {
			m_client->rate_limit_global(tp);
			std::this_thread::sleep_until(tp);
			r.state->res.clear();
			r.state->res.body().clear();
			send_rq(r);
		}else{
			r.state->res.clear();
			r.state->res.body().clear();
			//add the request to the right queue in order
			auto it = m_rate_limited_requests.insert(ranges::upper_bound(m_rate_limited_requests, tp, std::less<>{}, get_n<1>{}), 
													 { major_param_id_,tp,{} });
			std::get<2>(*it).push_back(std::move(r));
			return false;
		}
	}//this shuoldb't be riunning often ;-;

	
	if (auto it = r.state->res.find("X-RateLimit-Remaining"); it!=r.state->res.end()) {
		
		const auto time = std::chrono::system_clock::time_point(std::chrono::seconds([&](){
			int64_t seconds = 0;
			std::from_chars(it->value().begin(), it->value().end(), seconds);
			return seconds;
		}()));

		m_rate_limited_requests.insert(std::upper_bound(m_rate_limited_requests.begin(), m_rate_limited_requests.end(), time, [](const std::chrono::system_clock::time_point& a, const auto& b) {
			return a < std::get<1>(b);
		}), { major_param_id_,time,{} });
	}
	
	std::cout << r.req << std::endl;
	std::cout << r.state->res << std::endl;
	r.state->done = true;
	return true;
}

template<typename results_t>
boost::system::error_code connect_with_no_delay(boost::asio::ip::tcp::socket& sock,results_t&& results) {
	boost::system::error_code ec;
	for (auto&& thing : results) {
		sock.open(thing.endpoint().protocol());
		sock.set_option(boost::asio::ip::tcp::no_delay(true));;
		sock.connect(thing, ec);
		if (!ec)
			break;
		sock.close();
	}return ec;
}

void discord_http_connection::connect() {
	const auto results = m_resolver.resolve("discordapp.com", "https");
	//boost::asio::connect(m_ssl_stream.next_layer(), results.begin(), results.end());
	connect_with_no_delay(m_socket.next_layer(), results);
	m_socket.handshake(boost::asio::ssl::stream_base::client);
}

void discord_http_connection::reconnect() {	
	m_socket.next_layer().close();
	const auto results = m_resolver.resolve("discordapp.com", "https");
	connect_with_no_delay(m_socket.next_layer(), results);
	m_socket.handshake(boost::asio::ssl::stream_base::client);
}

void discord_http_connection::send_rq(discord_request& r) {
	boost::beast::error_code ec;
	boost::beast::http::write(m_socket, r.req, ec);
	if (ec)
		std::cout << "rawrrrrr " << ec << std::endl;
	boost::beast::http::read(m_socket, m_buffer, r.state->res, ec);
	if (ec)
		std::cout << "rawrrrrr " << ec << std::endl;
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
