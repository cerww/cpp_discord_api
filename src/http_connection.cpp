#include "client.h"
#include "http_connection.h"
#include <charconv>
#include <range/v3/core.hpp>
#include "../common/task_completion_handler.h"
#include "../common/executor_binder.h"

template<int i>
struct get_n {
	template<typename T>
	constexpr decltype(auto) operator()(T&& t) const {
		return std::get<i>(std::forward<T>(t));
	}
};

struct empty_vector_t{
	template<typename T,typename A>
	operator std::vector<T,A>() const{
		return {};
	}
};

static constexpr empty_vector_t empty_vector;

template<typename results_t>
cerwy::task<boost::system::error_code> async_connect_with_no_delay(boost::asio::ip::tcp::socket& sock, results_t&& results) {
	boost::system::error_code ec;
	for (auto&& thing : results) {
		sock.open(thing.endpoint().protocol());
		sock.set_option(boost::asio::ip::tcp::no_delay(true));
		ec = co_await sock.async_connect(thing, use_task_return_ec);
		if (!ec)
			break;
		sock.close();
	}
	co_return ec;
}

uint64_t get_major_param_id(std::string_view s) {
	//always first number after "/api/v6", or it's not included
	s.remove_prefix(7);//remove "/api/v6"
	const auto start = s.find_first_of("1234567890");
	if (start == std::string_view::npos)
		return 0;
	s.remove_prefix(start);
	const auto end = ranges::find(s, '/');
	const auto length = std::distance(s.begin(), end);
	uint64_t ret = 0;
	std::from_chars(s.data(), s.data() + length, ret);
	return ret;
}

//
// template<typename results_t>
// boost::system::error_code connect_with_no_delay(boost::asio::ip::tcp::socket& sock, results_t&& results) {
// 	boost::system::error_code ec;
// 	for (auto&& thing : results) {
// 		sock.open(thing.endpoint().protocol());
// 		sock.set_option(boost::asio::ip::tcp::no_delay(true));;
// 		sock.connect(thing, ec);
// 		if (!ec)
// 			break;
// 		sock.close();
// 	}
// 	return ec;
// }
//
//
//
//
//
// http_connection::http_connection(client* t,boost::asio::io_context& ioc):
// 	m_ioc(ioc),
// 	m_resolver(ioc),
// 	m_socket(ioc, m_sslCtx),
// 	m_client(t)
// {	
// 	
// 	
// 	
// }
//
// void http_connection::send_to_discord(discord_request r) {	
// 	const size_t major_param_id = get_major_param_id(std::string_view(r.req.target().data(), r.req.target().size()));
// 	const auto it = ranges::find(m_rate_limited_requests, major_param_id, get_n<0>{});
//
// 	if(it != m_rate_limited_requests.end()) {
// 		std::get<2>(*it).push_back(std::move(r));
// 		return;
// 	}
// 	if (m_global_rate_limited.load()) {
// 		std::this_thread::sleep_until(m_rate_limted_until);
// 		m_global_rate_limited.store(false);
// 	}
// 	if(send_to_discord_(r,major_param_id))
// 		r.state->notify();
// }
//
// //returns wether or not it's not local rate limited
// bool http_connection::send_to_discord_(discord_request& r,size_t major_param_id_) {
// 	std::lock_guard<std::mutex> locky(r.state->ready_mut);
// 	send_rq(r);
// 	//this is "while" loop not "if" because of global rate limits
// 	//it shuld keep trying to send the same request if it's global rate limited
// 	//cuz if i doin't do this, the request will go into the queue, then it'll sleep, then it'll prolyl send the request(it might sned a different request),
// 	//i can skip adding the request into the queue by doing this
// 	while (r.state->res.result_int() == 429) {
// 		std::cout << "rate limited" << '\n';
// 		nlohmann::json json_body = nlohmann::json::parse(r.state->res.body());
// 		const auto tp = std::chrono::system_clock::now() + std::chrono::milliseconds(json_body["retry_after"].get<size_t>());
// 		if(json_body["global"].get<bool>()) {
// 			m_client->rate_limit_global(tp);
// 			std::this_thread::sleep_until(tp);
// 			r.state->res.clear();
// 			r.state->res.body().clear();
// 			send_rq(r);
// 		}else{
// 			r.state->res.clear();
// 			r.state->res.body().clear();
// 			//add the request to the right queue in sorted order, by time
// 			auto it = m_rate_limited_requests.insert(ranges::upper_bound(m_rate_limited_requests, tp, std::less{}, get_n<1>{}), 
// 													{ major_param_id_,tp,empty_vector });
// 			std::get<2>(*it).push_back(std::move(r));
// 			return false;
// 		}
// 	}//this shuoldb't be riunning often ;-;
// 	
// 	if (auto it = r.state->res.find("X-RateLimit-Remaining"); it!=r.state->res.end()) {
// 		
// 		const auto time = std::chrono::system_clock::time_point(std::chrono::seconds([&](){//iife
// 			int64_t seconds = 0;
// 			std::from_chars(it->value().begin(), it->value().end(), seconds);
// 			return seconds;
// 		}()));
//
// 		m_rate_limited_requests.insert(ranges::upper_bound(m_rate_limited_requests, time, std::less{} ,get_n<1>{}),
// 									   { major_param_id_,time, empty_vector });
// 	}
// 	
// 	std::cout << r.req << std::endl;
// 	std::cout << r.state->res << std::endl;
// 	r.state->done = true;
// 	return true;
// }
//
// cerwy::task<boost::beast::error_code> http_connection::async_connect() {
// 	const auto[ec, results] = co_await m_resolver.async_resolve("discord.com", "https", use_task_return_tuple2);
// 	if(ec) {
// 		co_return ec;
// 	}
// 	const auto ec2  = co_await async_connect_with_no_delay(m_socket.next_layer(), results);
// 	if(ec2) {
// 		co_return ec2;
// 	}
// 	const auto ec3 = co_await m_socket.async_handshake(boost::asio::ssl::stream_base::client, use_task_return_ec);
// 	if(!ec3) {
// 		m_thread = std::thread([this]() {
// 			while (!m_done.load()) {
// 				if (m_rate_limited_requests.empty())
// 					send_to_discord(m_request_queue.pop());
// 				else if (std::optional<discord_request> r = m_request_queue.try_pop_until(std::get<1>(m_rate_limited_requests.front())); r) {
// 					send_to_discord(std::move(r.value()));
// 				} else {//std::chrono::system_clock::now() >= std::get<1>(m_rate_limited_requests[0])
// 					auto requests_to_send = std::move(m_rate_limited_requests[0]);
// 					m_rate_limited_requests.erase(m_rate_limited_requests.begin());
// 					for (auto& request : std::get<2>(requests_to_send))
// 						send_to_discord(std::move(request));
// 				}
// 			}
// 		});
// 	}
// 	co_return ec3;
// }
//
// void http_connection::connect() {
// 	auto results = m_resolver.resolve("discord.com", "https");
// 	connect_with_no_delay(m_socket.next_layer(), results);
// 	m_socket.handshake(boost::asio::ssl::stream_base::client);
// }
//
// void http_connection::reconnect() {	
// 	m_socket.next_layer().close();
// 	const auto results = m_resolver.resolve("discord.com", "https");
// 	connect_with_no_delay(m_socket.next_layer(), results);
// 	m_socket.handshake(boost::asio::ssl::stream_base::client);
// }
//
// void http_connection::send_rq(discord_request& r) {
// 	redo:
// 	boost::beast::error_code ec;
// 	boost::beast::http::write(m_socket, r.req, ec);
// 	if (ec) {		
// 		std::cout << "send_rq " << ec << std::endl;
// 		if(ec.value() == 100053) {
// 			reconnect();
// 		}
// 		goto redo;
// 	}
// 	boost::beast::http::read(m_socket, m_buffer, r.state->res, ec);
// 	if (ec) {		
// 		std::cout << "send_rq " << ec << std::endl;
// 		if (ec.value() == 1) {
// 			reconnect();
// 		}
// 		goto redo;
// 	}
// }
//

/*
template<typename Socket,typename Buffer>
void send_thingy(discord_request& r, Socket& sock, Buffer& buffer, client* parent) {
	std::cout << r.req << std::endl;
	
	std::lock_guard<std::mutex> locky(r.state->ready_mut);
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

	const auto results = resolver.resolve("discord.com", "https");
	boost::asio::connect(m_ssl_stream.next_layer(), results.begin(), results.end());
	m_ssl_stream.handshake(boost::asio::ssl::stream_base::client);

	for co_await(auto&& rq: hand) {
		std::visit([&](auto&& thing) {
			using type = std::decay_t<decltype(thing)>;
			if constexpr(std::is_same_v<type, discord_request>) {				
				send_thingy(thing, m_ssl_stream, m_buffer, parent);
				thing.state->ready_cv.notify_all();
			}
			else if constexpr(std::is_same_v<type, std::chrono::milliseconds>) {//
				std::this_thread::sleep_for(thing);
			}

		}, rq);
	}
}
*/

http_connection2::http_connection2(client* t, boost::asio::io_context& ioc) :
	m_ioc(ioc),
	m_resolver(ioc),
	m_socket(ioc, m_ssl_ctx),
	m_client(t),
	m_rate_limiter(ioc)
{
	m_rate_limiter.on_rate_limit_finish = [this](std::vector<discord_request> requests) {
		for(auto& request:requests) {
			if (!request.state->is_canceled) {
				send(std::move(request));
			}else {
				request.state->finish();
			}
		}
	};

}

void http_connection2::send(discord_request&& d) {
	m_request_queue.push(std::move(d));
	//send_to_discord(std::move(d));
}


cerwy::task<boost::beast::error_code> http_connection2::async_connect() {
	const auto [ec, results] = co_await m_resolver.async_resolve("discord.com", "https", use_task_return_tuple2);
	if (ec) {
		co_return ec;
	}
	const auto ec2 = co_await async_connect_with_no_delay(m_socket.next_layer(), results);
	if (ec2) {
		co_return ec2;
	}
	const auto ec3 = co_await m_socket.async_handshake(boost::asio::ssl::stream_base::client, use_task_return_ec);
	if(!ec3)
		start_sending();
	co_return ec3;
}

cerwy::task<void> http_connection2::send_to_discord(discord_request r) {
	return send_to_discord(r, get_major_param_id(std::string_view(r.req.target().data(), r.req.target().size())));
}

cerwy::task<void> http_connection2::send_to_discord(discord_request r, uint64_t major_param_id_) {
	//auto lock = co_await m_mut.async_lock();
	//i don't think i need this^ if i use the queue

	if(r.state->is_canceled) {
		r.state->finish();
		co_return;
	}
	
	if (m_global_rate_limited.load()) {
		if (m_rate_limted_until > std::chrono::system_clock::now()) {
			boost::asio::system_timer timer(m_ioc);
			timer.expires_at(m_rate_limted_until);
			co_await timer.async_wait(use_task);
		}
		m_global_rate_limited.store(false);
	}

	if (m_rate_limiter.maybe_rate_limit(major_param_id_, r)) {
		co_return;
	}

	std::lock_guard<std::mutex> locky(r.state->ready_mut);
	if (co_await send_to_discord_(r))
		r.state->finish();
}

//pre-con: request isn't rate-limited already
//false => rate limited
cerwy::task<bool> http_connection2::send_to_discord_(discord_request& r) {
	//std::lock_guard<std::mutex> locky(r.state->ready_mut);	
	co_await send_rq(r);	
	//this is "while" loop not "if" because of global rate limits
	//it shuld keep trying to send the same request if it's global rate limited
	//cuz if i doin't do this, the request will go into the queue, then it'll sleep, then it'll prolyl send the request(it might sned a different request),
	//i can skip adding the request into the queue by doing this
	while (r.state->res.result_int() == 429) {
		//std::cout << "rate limited" << '\n';

		if(r.state->is_canceled) {
			co_return true;
		}
		auto json_body = nlohmann::json::parse(r.state->res.body());
		const auto tp = std::chrono::system_clock::now() + std::chrono::milliseconds(json_body["retry_after"].get<size_t>());
		if (json_body["global"].get<bool>()) {
			
			m_client->rate_limit_global(tp);
			
			boost::asio::system_timer timer(m_ioc);
			timer.expires_at(tp);
			co_await timer.async_wait(use_task);
			
			r.state->res.clear();
			r.state->res.body().clear();
			co_await send_rq(r);
		} else {
			r.state->res.clear();
			r.state->res.body().clear();

			const auto major_param_id_ = get_major_param_id(std::string_view(r.req.target().data(), r.req.target().size()));			
			//rate_limit_id(major_param_id_, tp, std::move(r));
			m_rate_limiter.rate_limit(major_param_id_, tp, std::move(r));
			
			co_return false;
		}
	}
	
	
	if (auto it = r.state->res.find("X-RateLimit-Remaining"); it != r.state->res.end() && it->value() == "0") {
		
		const auto time = std::chrono::system_clock::time_point(std::chrono::seconds([&]() {//iife
			const auto it2 = r.state->res.find("X-RateLimit-Reset");
			int64_t seconds = 0;
			std::from_chars(it2->value().begin(), it2->value().end(), seconds);
			return seconds;
		}()));

		
		const auto major_param_id_ = get_major_param_id(std::string_view(r.req.target().data(), r.req.target().size()));
		//rate_limit_id(major_param_id_,time,std::nullopt);
		m_rate_limiter.rate_limit(major_param_id_, time);
	}

	std::cout << r.req << std::endl;
	std::cout << r.state->res << std::endl;	
	co_return true;
}

cerwy::task<void> http_connection2::start_sending() {
	while(true) {
		auto msg = co_await m_request_queue.pop();
		co_await send_to_discord(std::move(msg));
	}
}

// void http_connection2::resend_rate_limted_requests_for(uint64_t id) {
// 	//auto v = *ranges::lower_bound(m_rate_limited_requests, id, std::less(), get_n<1>());;
//
// 	auto requests = [&]() {//iife to guard mutex
// 		std::lock_guard lock(m_rate_limit_mut);
// 		auto ret = std::move(m_rate_limited_requests.front());
// 		m_rate_limited_requests.erase(m_rate_limited_requests.begin());
// 		return ret;
// 	}();
// 	
// 	for(auto& r: requests.requests) {
// 		send(std::move(r));
// 	}	
// }
//
// void http_connection2::rate_limit_id(uint64_t major_param_id_, std::chrono::system_clock::time_point tp, std::optional<discord_request> rq) {
// 	if(rq.has_value()) {
// 		std::lock_guard lock(m_rate_limit_mut);
// 		m_rate_limited_requests.insert(
// 			ranges::upper_bound(m_rate_limited_requests, tp, std::less{}, &rate_limit_entry::until),
// 			{ major_param_id_,tp, std::vector{std::move(rq.value())} }
// 		);
// 	}else {
// 		std::lock_guard lock(m_rate_limit_mut);
// 		m_rate_limited_requests.insert(
// 			ranges::upper_bound(m_rate_limited_requests, tp, std::less{}, &rate_limit_entry::until),
// 			{ major_param_id_,tp, empty_vector }
// 		);
// 	}
//
// 	auto timer = std::make_unique<boost::asio::system_timer>(m_ioc);
// 	timer->expires_at(tp);
// 	timer->async_wait([this, major_param_id_, t = std::move(timer)](auto ec)mutable {
// 		if (!ec)
// 			resend_rate_limted_requests_for(major_param_id_);
// 	});
// }
//
// //returns weather or not it was rate_limited
// bool http_connection2::check_rate_limit(uint64_t id, discord_request& rq) {
// 	std::lock_guard lock(m_rate_limit_mut);
// 	
// 	const auto it = ranges::find(m_rate_limited_requests, id, &rate_limit_entry::id);
// 	if (it != m_rate_limited_requests.end()) {
// 		auto& vec = it->requests;
// 		vec.push_back(std::move(rq));
// 		return true;
// 	}else {
// 		return false;
// 	}
// }

/*
void http_connection2::connect() {
	auto results = m_resolver.resolve("discord.com", "https");
	connect_with_no_delay(m_socket.next_layer(), results);
	m_socket.handshake(boost::asio::ssl::stream_base::client);
}
*/

cerwy::task<void> http_connection2::reconnect() {
	while (true) {
		if (m_socket.next_layer().is_open()) {
			co_await m_socket.async_shutdown(use_task);
		}
		const auto [ec, results] = co_await m_resolver.async_resolve("discord.com", "https", use_task_return_tuple2);
		if(ec) {
			continue;
		}
		auto ec1 = co_await async_connect_with_no_delay(m_socket.next_layer(), results);
		if(ec1) {
			continue;
		}
		auto [ec2] = co_await m_socket.async_handshake(boost::asio::ssl::stream_base::client, use_task_return_tuple2);
		if(ec2) {
			continue;
		}
		break;
	}
}

cerwy::task<void> http_connection2::send_rq(discord_request& request) {
	//TODO remove gotos
redo:
	auto [ec, n] = co_await boost::beast::http::async_write(m_socket, request.req, use_task_return_tuple2);

	if (ec) {
		std::cout << "send_rq " << ec << std::endl;
		if (ec.value() == 10053) {
			co_await reconnect();
			goto redo;
		}
	}
	auto[ec2,n2] = co_await boost::beast::http::async_read(m_socket, m_buffer, request.state->res, use_task_return_tuple2);
	if (ec2) {
		std::cout << "recieve_rq " << ec << std::endl;
		if (ec.value() == 1) {
			co_await reconnect();
			goto redo;
		}
	}
	
}
