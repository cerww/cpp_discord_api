#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "ssl_stream.hpp"
#include "task.h"

/*

template<typename T>
concept hub_trait = requires(T thing){
	T::socket_type;
	{thing.create_socket(boost::asio::ip::tcp::socket)}->T::socket_type;
	{co_await thing.init_socket(typename T::socket_type&)}->decltype(auto);
}


*/

struct basic_traits {
	using socket_type   = boost::asio::ip::tcp::socket;

	template<typename ...args>
	explicit basic_traits(args&&...) {};

	static std::experimental::suspend_never init_socket(socket_type& s) {
		return {};
	}

	static socket_type create_socket(boost::asio::ip::tcp::socket r) {
		return std::move(r);
	}
};

template<boost::asio::ssl::context_base::method method>
struct ssl_traits {
	using socket_type = ssl_stream<boost::asio::ip::tcp::socket>;	
	
	template<typename parent>
	explicit ssl_traits(parent& p) :m_type((boost::asio::ssl::stream_base::handshake_type)p.hub_type()){
		
	};
	
	socket_type create_socket(boost::asio::ip::tcp::socket r) {
		socket_type ret(std::move(r),m_ssl_ctx);
		return ret;
	}

	task<void> init_socket(socket_type& s) {
		co_await s.async_handshake(m_type, use_task);
	}

private:
	boost::asio::ssl::context m_ssl_ctx{ method };
	boost::asio::ssl::stream_base::handshake_type m_type;
};


