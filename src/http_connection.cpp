#include "http_connection.h"
#include <charconv>
#include "../common/task_completion_handler.h"
#include "../common/executor_binder.h"
#include <nlohmann/json.hpp>
#include <iostream>

template<int i>
struct get_n {
	template<typename T>
	constexpr decltype(auto) operator()(T&& t) const {
		return std::get<i>(std::forward<T>(t));
	}
};

template<typename results_t>
cerwy::eager_task<boost::system::error_code> async_connect_with_no_delay(boost::asio::ip::tcp::socket& sock, results_t&& results) {
	boost::system::error_code ec;
	for (auto&& thing : results) {
		sock.open(thing.endpoint().protocol());
		sock.set_option(boost::asio::ip::tcp::no_delay(true));
		ec = co_await sock.async_connect(thing, use_task_return_ec);
		if (!ec) {
			break;
		}
		sock.close();
	}
	co_return ec;
}

uint64_t get_major_param_id(std::string_view s) {
	//always first number after "/api/v8", or it's not included
	s.remove_prefix(7);//remove "/api/v8"
	const auto start = s.find_first_of("0123456789");
	if (start == std::string_view::npos) {
		return 0;
	}
	s.remove_prefix(start);
	const auto end = std::ranges::find(s, '/');
	const auto length = end - s.begin();
	uint64_t ret;
	std::from_chars(s.data(), s.data() + length, ret);
	return ret;
}

http_connection2::http_connection2(boost::asio::io_context& ioc) :
	m_ioc(ioc),
	m_resolver(ioc),
	m_socket(ioc, m_ssl_ctx),
	m_rate_limiter(ioc)
{
	m_rate_limiter.on_rate_limit_finish = [this](std::vector<discord_request> requests) {
		for(auto& request:requests) {			
			send(std::move(request));			
		}
	};

}

void http_connection2::send(discord_request&& d) {
	m_request_queue.push(std::move(d));
	//send_to_discord(std::move(d));
}

cerwy::eager_task<boost::beast::error_code> http_connection2::async_connect() {
	const auto [ec, results] = co_await m_resolver.async_resolve("discord.com", "https", use_task_return_tuple2);
	if (ec) {
		co_return ec;
	}
	const auto ec2 = co_await async_connect_with_no_delay(m_socket.next_layer(), results);
	if (ec2) {
		co_return ec2;
	}
	const auto ec3 = co_await m_socket.async_handshake(boost::asio::ssl::stream_base::client, use_task_return_ec);	
	if (!ec3) {
		start_sending();
	}
	co_return ec3;
}

cerwy::lazy_task<void> http_connection2::send_to_discord(discord_request r) {
	const auto id = get_major_param_id(std::string_view(r.req.target().data(), r.req.target().size()));//can't be inline since it'll be use after move
	return send_to_discord(std::move(r),id);
}

cerwy::lazy_task<void> http_connection2::send_to_discord(discord_request r, uint64_t major_param_id_) {
	//auto lock = co_await m_mut.async_lock();
	//i don't think i need this^ if i use the queue	
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

	//std::lock_guard<std::mutex> locky(r.state->ready_mut);
	boost::beast::http::response<boost::beast::http::string_body> res;
	if (co_await send_to_discord_(r, res)) {
		if (r.on_finish.has_value()) {
			(*r.on_finish)(std::move(res));
		}
	}
}

//pre-con: request isn't rate-limited already
//false => rate limited
cerwy::lazy_task<bool> http_connection2::send_to_discord_(discord_request& r, boost::beast::http::response<boost::beast::http::string_body>& res) {
	co_await send_rq(r,res);
	//this is "while" loop not "if" because of global rate limits
	//it shuld keep trying to send the same request if it's global rate limited
	//cuz if i doin't do this, the request will go into the queue, then it'll sleep, then it'll prolyl send the request(it might sned a different request),
	//i can skip adding the request into the queue by doing this
	while (res.result_int() == 429) {
		//std::cout << "rate limited" << '\n';
		
		auto json_body = nlohmann::json::parse(res.body());
		const auto tp = std::chrono::system_clock::now() + std::chrono::milliseconds((uint64_t)json_body["retry_after"].get<double>());
		if (json_body["global"].get<bool>()) {
			
			//m_client->rate_limit_global(tp);
			m_on_global_ratelimit(tp);
			
			boost::asio::system_timer timer(m_ioc);
			timer.expires_at(tp);
			co_await timer.async_wait(use_task);
			
			res.clear();
			res.body().clear();
			co_await send_rq(r,res);
		} else {
			res.clear();
			res.body().clear();

			const auto major_param_id_ = get_major_param_id(std::string_view(r.req.target().data(), r.req.target().size()));			
			m_rate_limiter.rate_limit(major_param_id_, tp, std::move(r));
			
			co_return false;
		}
	}	
	
	if (const auto it = res.find("X-RateLimit-Remaining"); it != res.end() 
		&& it->value() == "0") 	{
		
		const auto time = std::chrono::system_clock::time_point(std::chrono::milliseconds([&]() {//iife
			const auto it2 = res.find("X-RateLimit-Reset-After");
			double milliseconds = 0;
			std::from_chars(it2->value().begin(), it2->value().end(), milliseconds);
			return (uint64_t)milliseconds;
		}()) + std::chrono::system_clock::now());
		
		const auto major_param_id_ = get_major_param_id(std::string_view(r.req.target().data(), r.req.target().size()));
		m_rate_limiter.rate_limit(major_param_id_, time);
	}

	std::cout << r.req << std::endl;
	std::cout << res << std::endl;	
	co_return true;
}

cerwy::eager_task<void> http_connection2::start_sending() {
	while(true) {
		auto msg = co_await m_request_queue.pop();
		co_await send_to_discord(std::move(msg));
	}
}

cerwy::lazy_task<void> http_connection2::reconnect() {
	while (true) {
		if (m_socket.next_layer().is_open()) {			
			auto [ec] = co_await m_socket.async_shutdown(use_task_return_tuple2);
			if(ec) {
				//ignore, might be open but hasn't done handshake yet
			}			
			m_socket.next_layer().close(ec);			
		}
		const auto [ec, results] = co_await m_resolver.async_resolve("discord.com", "https", use_task_return_tuple2);
		if(ec) {
			continue;
		}
		auto [ec1,r] = co_await boost::asio::async_connect(m_socket.next_layer(), results,use_task_return_tuple2);
		//auto ec1 = co_await async_connect_with_no_delay(m_socket.next_layer(), results);
		if(ec1) {
			continue;
		}
		auto [ec2] = co_await m_socket.async_handshake(boost::asio::ssl::stream_base::client, use_task_return_tuple2);
		if(ec2) {
			continue;
		}
		co_return;
	}
}

cerwy::lazy_task<void> http_connection2::send_rq(discord_request& request,boost::beast::http::response<boost::beast::http::string_body>& res) {
	//TODO remove gotos
redo:
	auto [ec, n] = co_await boost::beast::http::async_write(m_socket, request.req, use_task_return_tuple2);

	if (ec) {
		std::cout << request.req << std::endl;
		std::cout << "send_rq " << ec << std::endl;
		if(ec.category() == boost::asio::error::get_ssl_category()) {
			std::string err = std::string(" (")
				+ std::to_string(ERR_GET_LIB(ec.value())) + ","
				+ std::to_string(ERR_GET_FUNC(ec.value())) + ","
				+ std::to_string(ERR_GET_REASON(ec.value())) + ") ";
			std::cout << err << std::endl;
		}
		co_await reconnect();
		goto redo;
	}
	auto[ec2,n2] = co_await boost::beast::http::async_read(m_socket, m_buffer, res, use_task_return_tuple2);
	if (ec2) {
		std::cout << "recieve_rq " << ec << std::endl;
		co_await reconnect();
		res.clear();
		res.body().clear();
		goto redo;
	}
	
}
