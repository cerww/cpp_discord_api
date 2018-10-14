#pragma once
#include <boost/asio.hpp>
#include "stuff.h"
#include "web_socket.h"
#include "networking_coro_stuff.h"
#include <boost/beast.hpp>

template<typename s1,typename s2>
bool cistr_comp(s1&& a,s2&& b) {
	return a.size() == b.size() && std::inner_product(a.cbegin(), a.cend(), b.cbegin(), 0, [](char a,char b){
		return to_lower(a) != to_lower(b);
	}, std::plus<>{})==0;
}

inline bool is_valid_request(const boost::beast::http::request<boost::beast::http::string_body>& rq) {
	try {
		return 
			rq.version() == 11 &&
			!rq.count(boost::beast::http::field::host) &&
			rq.method() == boost::beast::http::verb::get &&
			cistr_comp(rq.at(boost::beast::http::field::connection),"upgrade"sv)&&
			cistr_comp(rq.at(boost::beast::http::field::upgrade), "websocket"sv)&&
			rq.at("Sec-WebSocket-Version") == "13";		
	}catch(...) {
		return false;
	}return true;
}

struct ipv4{
	static auto ip_version() {
		return boost::asio::ip::tcp::v4();
	}
};

struct ipv6{
	static auto ip_version() {
		return boost::asio::ip::tcp::v6();
	}
};

template<typename traits>
struct server_hub_no_ioc:private traits, hub_base<server_hub_no_ioc<traits>, web_socket<typename traits::socket_type, server_hub_no_ioc<traits>>> {
	using my_base = traits;
	using lower_socket_type = typename traits::socket_type;
	using web_socket_type = web_socket<lower_socket_type, server_hub_no_ioc<traits>>;
	using resolver_type = boost::asio::ip::tcp::resolver;
	using acceptor_type = boost::asio::ip::tcp::acceptor;

	static constexpr hub_type_t hub_type() {
		return hub_type_t::server;
	}

	static constexpr bool is_client() noexcept { return false; }

	explicit server_hub_no_ioc(boost::asio::io_context& ioc):
		traits(*this),
		m_ioc(ioc),
		m_acceptor(ioc) {}

	boost::system::error_code bind(const boost::asio::ip::tcp::endpoint& end) {
		boost::system::error_code ec;
		m_acceptor.open(end.protocol());		
		m_acceptor.set_option(boost::asio::socket_base::reuse_address(true));
		m_acceptor.set_option(boost::asio::ip::tcp::no_delay(true));
		m_acceptor.bind(end,ec);
		m_acceptor.listen();		
		return ec;
	}

	template<typename ipv_t>
	boost::system::error_code bind(int port,ipv_t&& ip_version) {
		boost::asio::ip::tcp::endpoint ep(ip_version.ip_version(),port);
		return bind(ep);
	}

	void listen() {
		m_acceptor.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket s) {
			if (ec) std::cout << ec << std::endl;
			else {
				listen();
				on_accept(std::move(s), std::move(ec));
			};
		});
	}

	auto& get_io_context() noexcept {
		return m_ioc;
	}

	const auto& get_io_context() const noexcept {
		return m_ioc;
	}

private:
	task<void> on_accept(boost::asio::ip::tcp::socket s,boost::system::error_code ec) {
		try{
			s.set_option(boost::asio::ip::tcp::no_delay(true));			
			auto sock = std::make_shared<web_socket_type>(this->create_socket(std::move(s)), this);
			co_await this->init_socket(sock->socket().next_layer());
			co_await sock->socket().async_accept(use_task);			
						
			this->on_connect(*sock);			
			sock->start_reads();
		}catch(...) {
			std::cout << "hat" << std::endl;			
		}//socket dies if there's an error
	}

	boost::asio::io_context& m_ioc;
	acceptor_type m_acceptor;
};

template<typename traits>
struct server_hub :
		has_ioc,//if i put ioc in this class, it'll call the other constructor before the ioc constructor
		server_hub_no_ioc<traits> {
	server_hub() :server_hub_no_ioc<traits>(this->p_get_ioc()) {}

	server_hub(const server_hub&) = delete;
	server_hub& operator=(const server_hub&) = delete;

	server_hub(server_hub&&) = delete;
	server_hub& operator=(server_hub&&) = delete;

	~server_hub() {
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





