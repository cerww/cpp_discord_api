#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <experimental/coroutine>
#include "web_socket.h"
#include <boost/asio/ssl.hpp>
#include "hub_traits.h"
#include "randomThings.h"
#include "networking_coro_stuff.h"
#include "task.h"
#include "task_completion_handler.h"

//258EAFA5-E914-47DA-95CA-sC5AB0DC85B11
using namespace std::literals;

constexpr std::tuple<std::string_view, std::string_view,std::string_view> parse_uri(std::string_view uri) {
	const auto split_point = uri.find_first_of(":/\\");
	const auto port = uri.substr(0,split_point);
	uri.remove_prefix(split_point);
	uri.remove_prefix(std::distance(uri.begin(), std::find_if(uri.begin(), uri.end(), [](const char c) {return is_letter(c) || is_number(c); })));

	std::string_view path = uri.substr(std::min(uri.find('/'), uri.size() - 1));	
	uri.remove_suffix(path.size());

	return { port,uri, path};
}

inline auto create_handshake_rq(std::string_view uri,std::string_view path) {
	boost::beast::http::request<boost::beast::http::string_body> rq(boost::beast::http::verb::get, std::string(path), 11);
	rq.set("Sec-WebSocket-Key", "dGhlIHNhbXBsZSBub25jZQ==");
	rq.set("Host", uri);
	rq.set("Connection", "Upgrade");	
	rq.set("Origin", uri);
	rq.set("Sec-WebSocket-Protocol", "chat");
	rq.set("Sec-WebSocket-Version", "13");
	rq.set("Upgrade", "websocket");
	return rq;
}

template<typename rng>
auto create_rq(std::string_view uri,std::string_view path,std::string body,rng&& extra_headers) {
	auto rq = create_handshake_rq(uri, path);
	rq.body() = std::string(body);
	for (auto&& i : extra_headers)
		rq.set(std::string(i.first), std::string(i.second));
	return rq;
}

template<typename socket_t,typename endpt_seq>
task<boost::system::error_code> connect_socket(socket_t& sock,endpt_seq&& endpoints) {	
	boost::system::error_code ec;
	for (auto&& r : endpoints) {
		sock.open(r.endpoint().protocol());
		sock.set_option(boost::asio::ip::tcp::no_delay(true));
		ec = co_await sock.async_connect(r.endpoint(), use_task_return_ec);
		if (!ec)
			break;
		sock.close();
	}
	co_return ec;
}

template<typename traits>
struct client_hub_no_ioc :private traits,hub_base<client_hub_no_ioc<traits>, web_socket<typename traits::socket_type, client_hub_no_ioc<traits>>> {
	using my_base = traits;
	using lower_socket_type = typename traits::socket_type;
	using web_socket_type = web_socket<lower_socket_type,client_hub_no_ioc<traits>>;
	using resolver_type = boost::asio::ip::tcp::resolver;

	static constexpr hub_type_t hub_type() {
		return hub_type_t::client;
	}

	static constexpr bool is_client() noexcept{ return true; }

	explicit client_hub_no_ioc(boost::asio::io_context& ioc) :
	traits(*this),
	m_ioc(ioc),
	m_resolver(ioc) {}

	client_hub_no_ioc(const client_hub_no_ioc&) = delete;
	client_hub_no_ioc& operator=(const client_hub_no_ioc&) = delete;

	client_hub_no_ioc(client_hub_no_ioc&&) = delete;
	client_hub_no_ioc& operator=(client_hub_no_ioc&&) = delete;
	~client_hub_no_ioc() = default;
	
	template<typename rng,typename string_t>
	[[maybe_unused]]
	std::future<web_socket_type*> async_connect(std::string rawr,rng&& extra_headers,string_t&& body) {
		to_not_sso_string(rawr);
		
		const auto[port, uri,target] = parse_uri(rawr);
		std::promise<web_socket_type*> p;
		std::future<web_socket_type*> ret = p.get_future();
		m_resolver.async_resolve(uri, port, [this, port, uri, target, promise_ = std::move(p),extra_headers_ = std::move(extra_headers),body_ = std::forward<string_t>(body),pin = std::move(rawr)](const auto& ec, auto res)mutable {						
			if(!ec) {
				m_on_resolve(res, port, uri, target, std::move(promise_), std::move(extra_headers_), std::move(body_), std::move(pin));
			}else {
				try {
					throw boost::system::system_error(ec);
				}catch(...) {
					promise_.set_exception(std::current_exception());
				}
			}
		});
		return ret;
	}

	[[maybe_unused]]
	std::future<web_socket_type*> async_connect(std::string rawr) {
		return async_connect(std::move(rawr),std::vector<std::pair<std::string_view, std::string_view>>() , ""sv);
	}

	auto& get_io_context() noexcept{
		return m_ioc;
	}

	const auto& get_io_context() const noexcept {
		return m_ioc;
	}

private:
	template<typename endpt,typename rng,typename string_t>
	task<void> m_on_resolve(endpt&& end_points, std::string_view port, std::string_view uri,std::string_view target,std::promise<web_socket_type*> promise,rng extra_headers,string_t&& body,std::string full_url) {
		try{
			boost::asio::ip::tcp::socket sockland{ m_ioc };
					
			if (auto ec = co_await connect_socket(sockland, std::forward<endpt>(end_points)); ec)
				throw boost::system::system_error(ec);
			
			auto socky = std::make_shared<web_socket_type>(this->create_socket(std::move(sockland)),this);
			co_await this->init_socket(socky->socket().next_layer());//future requires default constructable, sockets aren't default constructablke ;-;

			co_await socky->socket().async_handshake_ex(std::string(uri), std::string(target), [&](auto& rq){
				for (auto [field, value] : extra_headers)
					rq.set(std::string(std::move(field)), std::string(std::move(value)));
			}, use_task);
			
			promise.set_value(socky.get());
			this->on_connect(*socky);
			socky->start_reads();
		}catch(...) {
			std::cout << "r;-=;" << std::endl;
			promise.set_exception(std::current_exception());
		}
	}
	boost::asio::io_context& m_ioc;
	resolver_type m_resolver;
};

template<typename traits>
struct client_hub: 
		has_ioc,//if i put ioc in this class, it'll call the other constructor before the ioc constructor
		client_hub_no_ioc<traits>{
	client_hub():client_hub_no_ioc<traits>(this->p_get_ioc()){}

	client_hub(const client_hub&) = delete;
	client_hub& operator=(const client_hub&) = delete;

	client_hub(client_hub&&) = delete;
	client_hub& operator=(client_hub&&) = delete;

	~client_hub() {
		m_done.store(true);
		this->p_get_ioc().stop();
	}

	void run() {
		auto temp = boost::asio::make_work_guard(this->p_get_ioc());
		this->p_get_ioc().run();		
	}

private:
	
	std::atomic<bool> m_done = false;
};



























