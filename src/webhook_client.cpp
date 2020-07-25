#include "webhook_client.h"
#include "task_completion_handler.h"


void webhook_http_client::send(webhook_request&& d) {
	m_request_queue.push(std::move(d));
}

cerwy::task<boost::beast::error_code> webhook_http_client::async_connect() {
	boost::asio::ip::tcp::resolver resolver(m_client->ioc());
	const auto [ec, results] = co_await resolver.async_resolve("discord.com", "https", use_task_return_tuple2);
	if (ec) {
		co_return ec;
	}
	const auto [ec2,ep] = co_await boost::asio::async_connect(m_socket.next_layer(), results,use_task_return_tuple2);
	if (ec2) {
		co_return ec2;
	}
	const auto ec3 = co_await m_socket.async_handshake(boost::asio::ssl::stream_base::client, use_task_return_ec);
	if (!ec3)
		start_sending();
	co_return ec3;
}

cerwy::task<void> webhook_http_client::send_to_discord(webhook_request r) {
	//don't need to check rate limit,if we're rate limted, we'll just get 429

	std::lock_guard<std::mutex> locky(r.state->ready_mut);
	if (co_await send_to_discord_(r))
		r.state->notify();
}

cerwy::task<bool> webhook_http_client::send_to_discord_(webhook_request& r) {
	co_await send_rq(r);
	
	while (r.state->res.result_int() == 429) {
		//std::cout << "rate limited" << '\n';
		auto json_body = nlohmann::json::parse(r.state->res.body());
		const auto tp = std::chrono::system_clock::now() + std::chrono::milliseconds(json_body["retry_after"].get<size_t>());


		boost::asio::system_timer timer(m_client->ioc());
		timer.expires_at(tp);
		co_await timer.async_wait(use_task);
		
		r.state->res.clear();
		r.state->res.body().clear();
		co_await send_rq(r);
	}

	std::cout << r.req << std::endl;
	std::cout << r.state->res << std::endl;
	r.state->done = true;
	co_return true;
}

cerwy::task<void> webhook_http_client::start_sending() {
	while(true) {
		auto rq = co_await m_request_queue.pop();
		(void)co_await send_to_discord(std::move(rq));
	}
}
cerwy::task<void> webhook_http_client::reconnect() {
	boost::asio::ip::tcp::resolver resolver(m_client->ioc());
	
	while (true) {
		if (m_socket.next_layer().is_open()) {
			co_await m_socket.async_shutdown(use_task);
		}
		const auto [ec, results] = co_await resolver.async_resolve("discord.com", "https", use_task_return_tuple2);
		if (ec) {
			continue;
		}
		auto [ec1,ep] = co_await boost::asio::async_connect(m_socket.next_layer(), results, use_task_return_tuple2);
		if (ec1) {
			continue;
		}
		auto [ec2] = co_await m_socket.async_handshake(boost::asio::ssl::stream_base::client, use_task_return_tuple2);
		if (ec2) {
			continue;
		}
		break;
	}
}

cerwy::task<void> webhook_http_client::send_rq(webhook_request& request) {
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
	auto [ec2, n2] = co_await boost::beast::http::async_read(m_socket, m_buffer, request.state->res, use_task_return_tuple2);
	if (ec2) {
		std::cout << "recieve_rq " << ec << std::endl;
		if (ec.value() == 1) {
			co_await reconnect();
			goto redo;
		}
	}
}
