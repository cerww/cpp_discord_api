#pragma once
#include "webhook.h"
#include <string>
#include "requests.h"
#include "task.h"
#include "concurrent_async_queue.h"
#include <boost/asio/ssl.hpp>
#include "ref_or_inplace.h"
#include "allowed_mentions.h"

struct webhook_request {
	boost::beast::http::request<boost::beast::http::string_body> req;
	ref_count_ptr<rq::shared_state> state = nullptr;
};

struct webhook_client_impl;

struct webhook_http_client {
	explicit webhook_http_client(webhook_client_impl* c, boost::asio::io_context& ioc):
		m_client(c),
		m_socket(ioc, m_ssl_ctx) { };

	void send(webhook_request&& d);

	void stop() {
		m_done.store(true);
	}

	cerwy::task<boost::beast::error_code> async_connect();

private:
	webhook_client_impl* m_client = nullptr;
	std::atomic<bool> m_done = false;

	boost::beast::flat_buffer m_buffer{};
	mpsc_concurrent_async_queue<webhook_request> m_request_queue = {};


	boost::asio::ssl::context m_ssl_ctx{boost::asio::ssl::context::tlsv12_client};
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_socket;

	cerwy::task<void> send_to_discord(webhook_request);//major param id is always webhook id

	cerwy::task<bool> send_to_discord_(webhook_request&);

	cerwy::task<void> start_sending();

	cerwy::task<void> reconnect();

	cerwy::task<void> send_rq(webhook_request&);
};

struct webhook_client_impl {

	webhook_client_impl(snowflake id, std::string token, boost::asio::io_context& ioc) :
		m_id(id),
		m_token(std::move(token)),
		m_ioc(ioc),
		m_strand(m_ioc.value()),
		m_http_connection(this, m_ioc.value()) {

		m_http_connection.async_connect();
	}

	webhook_client_impl(snowflake id, std::string token, boost::asio::io_context::strand& strand) :
		m_id(id),
		m_token(std::move(token)),
		m_ioc(strand.context()),
		m_strand(strand),
		m_http_connection(this, m_ioc.value()) {

		m_http_connection.async_connect();
	}

	webhook_client_impl(snowflake id, std::string token) :
		m_id(id),
		m_token(std::move(token)),
		m_ioc(1),
		m_strand(m_ioc.value()),
		m_http_connection(this, m_ioc.value()) {

		m_http_connection.async_connect();
	}

	//rq::execute_webhook send();

	boost::asio::io_context& ioc() {
		return m_ioc.value();
	}

	boost::asio::io_context::strand& strand() {
		return m_strand.value();
	}

	snowflake id() const noexcept { return m_id; };


	rq::execute_webhook send(std::string stuff) {
		nlohmann::json json;
		json["content"] = std::move(stuff);
		return send_request<rq::execute_webhook>(json.dump(), m_id, m_token);
	}

private:
	snowflake m_id;
	std::string m_token{};

	std::optional<webhook> m_webhook = std::nullopt;

	ref_or_inplace<boost::asio::io_context> m_ioc;
	ref_or_inplace<boost::asio::io_context::strand> m_strand;

	webhook_http_client m_http_connection;

	static void set_up_request(boost::beast::http::request<boost::beast::http::string_body>& req) {
		req.set("Application", "cerwy");
		req.set(boost::beast::http::field::host, "discord.com"s);
		req.set(boost::beast::http::field::user_agent, "watland");
		req.set(boost::beast::http::field::accept, "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
		req.set(boost::beast::http::field::accept_language, "en-US,en;q=0.5");
		req.keep_alive(true);
	}

	template<typename T, typename...args>
	std::enable_if_t<!rq::has_content_type_v<T>, T> send_request(args&&... Args);

	template<typename T, typename...args>
	std::enable_if_t<rq::has_content_type_v<T>, T> send_request(std::string&&, args&&... Args);

};

namespace rawrland2 {//rename later ;-;

	template<typename fut_type, typename ... Args>
	std::pair<fut_type, webhook_request> get_default_stuffs_for_request(Args&&... args) {
		std::pair<fut_type, webhook_request> retVal;
		retVal.second.state = make_ref_count_ptr<rq::shared_state>();

		retVal.first = fut_type(retVal.second.state);
		retVal.second.req = fut_type::request(std::forward<Args>(args)...);

		if constexpr (rq::has_content_type_v<fut_type>) {
			retVal.second.req.set(boost::beast::http::field::content_type, fut_type::content_type);
		}
		return retVal;
	}

}

template<typename T, typename ... args>
std::enable_if_t<rq::has_content_type_v<T>, T> webhook_client_impl::send_request(std::string&& body, args&&... Args) {
	auto [retVal, r] = rawrland2::get_default_stuffs_for_request<T>(std::forward<args>(Args)...);
	set_up_request(r.req);
	r.req.body() = std::move(body);
	r.req.prepare_payload();
	r.state->strand = &m_strand.value();
	m_http_connection.send(std::move(r));
	return std::move(retVal);//no nrvo cuz structured bindings
}

template<typename T, typename ... args>
std::enable_if_t<!rq::has_content_type_v<T>, T> webhook_client_impl::send_request(args&&... Args) {
	auto [retVal, r] = rawrland2::get_default_stuffs_for_request<T>(std::forward<args>(Args)...);
	set_up_request(r.req);
	r.req.prepare_payload();
	r.state->strand = &m_strand.value();
	m_http_connection.send(std::move(r));
	return std::move(retVal);//no nrvo cuz structured bindings
}

//needs to be movable so it can be assigned to
struct webhook_client {


	webhook_client(snowflake id, std::string token, boost::asio::io_context& ioc) :
		m_impl(std::make_unique<webhook_client_impl>(id, std::move(token), ioc)) { }

	webhook_client(snowflake id, std::string token, boost::asio::io_context::strand& strand) :
		m_impl(std::make_unique<webhook_client_impl>(id, std::move(token), strand)) { }

	webhook_client(snowflake id, std::string token) :
		m_impl(std::make_unique<webhook_client_impl>(id, std::move(token))) { }

	boost::asio::io_context& ioc() {
		return m_impl->ioc();
	}

	boost::asio::io_context::strand& strand() {
		return m_impl->strand();
	}

	snowflake id() const noexcept { return m_impl->id(); };


	rq::execute_webhook send(std::string stuff) {
		return m_impl->send(std::move(stuff));
	}

	void run() {
		ioc().run();
	}

private:
	std::unique_ptr<webhook_client_impl> m_impl = nullptr;
};
