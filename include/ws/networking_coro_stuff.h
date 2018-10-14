#pragma once
#include <boost/asio.hpp>
#include <experimental/coroutine>
#include <boost/beast.hpp>
namespace async_coro{

struct awaitable_base{
	static bool await_ready() noexcept{ return false; }
	boost::system::error_code await_resume() { return std::move(m_ec); }
protected:
	boost::system::error_code m_ec;
};

template<typename socket,typename end_point>
auto async_connect(socket& sock,end_point& p) {
	struct awaitable:awaitable_base {
		void await_suspend(std::experimental::coroutine_handle<> h) {
			boost::asio::async_connect(s, m_p, [this , h_ = h](auto ec, auto&&...){
				m_ec = std::move(ec);
				h_.resume();
			});
		}
		socket& s;
		end_point& m_p;
		boost::system::error_code m_ec;
	};
	return awaitable{{}, sock,p };
}

template<typename socket, typename buffer,typename = std::void_t<decltype(boost::asio::async_write(std::declval<socket>(), std::declval<buffer>(), [](){}))>>
auto async_write(socket& sock, buffer& thing) {
	struct awaitable:awaitable_base {
		void await_suspend(std::experimental::coroutine_handle<> h) {
			boost::asio::async_write(s, b, [this,h_= h](auto ec,size_t) {
				m_ec = std::move(ec);
				h_.resume();
			});
		}
		socket& s;
		buffer& b;
	};
	return awaitable{{}, sock,thing };
}


template<typename socket, typename buffer>
auto async_read(socket& sock, buffer& thing) {
	struct awaitable :awaitable_base {
		void await_suspend(std::experimental::coroutine_handle<> h) {
			boost::asio::async_read(s, b, [this, h_ = h](auto ec, size_t) {
				m_ec = std::move(ec);
				h_.resume();
			});
		}
		socket& s;
		buffer& b;
	};
	return awaitable{ {}, sock,thing };
}

template<typename socket, typename stuff>
auto http_async_write(socket& sock, stuff thing) {
	struct awaitable :awaitable_base {
		void await_suspend(std::experimental::coroutine_handle<> h) {
			boost::beast::http::async_write(s, b, [this,h_ = h](auto ec,size_t) {
				m_ec = std::move(ec);
				h_.resume();
			});
		}
		socket& s;
		stuff b;
	};
	return awaitable{ {}, sock,std::move(thing)};
}

template<typename socket, typename buffer_t,typename response>
auto http_async_read(socket& sock, buffer_t& buffer,response& r) {
	struct awaitable :awaitable_base {
		void await_suspend(std::experimental::coroutine_handle<> h) {
			boost::beast::http::async_read(s, b, r, [this, h_ = h](auto ec, size_t) {
				m_ec = std::move(ec);
				h_.resume();
			});
		}
		socket& s;
		buffer_t& b;
		response& r;
	};
	return awaitable{ {}, sock,buffer,r };
}

	template<typename socket,typename decorator>
	auto async_handshake_ex(socket& s,std::string host,std::string target,decorator&& d) {
		struct awaitable:awaitable_base{

			void await_suspend(std::experimental::coroutine_handle<> h) {
				sock.async_handshake_ex(host,target,decor,[this,h_= h](auto ec,auto&&...)mutable{
					this->m_ec = std::move(ec);
					h_.resume();
				});
			}

			socket& sock;
			std::string host;
			std::string target;
			decorator decor;
		};
		return awaitable{ {},s,std::move(host),std::move(target),d };
	}


}