#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <thread>
#include "Awaitables.h"
#include "concurrent_queue.h"
//#include <variant>

struct discord_request {
	boost::beast::http::request<boost::beast::http::string_body> req;
	ref_count_ptr<rq::shared_state> state;
};

class client;

class discord_http_connection{
public:
	void sleep_till(std::chrono::steady_clock::time_point time_point) {
		m_rate_limted_until = time_point;
		m_global_rate_limited.store(true);
	};
	discord_http_connection() = delete;
	discord_http_connection(discord_http_connection&&) = delete;
	discord_http_connection(const discord_http_connection&) = delete;
	discord_http_connection& operator=(const discord_http_connection&) = delete;
	discord_http_connection& operator=(discord_http_connection&&) = delete;

	explicit discord_http_connection(client*);

	~discord_http_connection() {
		m_done.store(true);
		if (m_thread.joinable())
			m_thread.join();
	};
	void add(discord_request&& d) {
		m_request_queue.push(d);
	}
	void stop() {
		m_done = true;
	}
private:
	std::chrono::steady_clock::time_point m_rate_limted_until = {};
	std::atomic<bool> m_global_rate_limited = false;

	std::atomic<bool> m_done = false;
	std::thread m_thread;

	boost::asio::io_context m_ioc;
	boost::asio::ip::tcp::resolver resolver{ m_ioc };

	boost::asio::ssl::context m_sslCtx{ boost::asio::ssl::context::tlsv12_client };
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_ssl_stream{ m_ioc, m_sslCtx };
	boost::beast::flat_buffer m_buffer;//needed cuz read_some can read more than it should
	concurrent_queue<discord_request> m_request_queue = {};
	client* m_client = nullptr;
	void send_to_discord(discord_request);
	void send_to_discord_(discord_request&, size_t);
	std::vector<std::tuple<size_t, std::chrono::system_clock::time_point, std::vector<discord_request>>> m_rate_limited;
};

/*
#include <experimental/coroutine>

struct coro_discord_http_connection{
	struct promise_type{
		std::experimental::suspend_never initial_suspend() {
			return {};
		}
		std::experimental::suspend_never final_suspend() {
			return {};
		}
		void unhandled_exception() {
			
		}

	};

	std::experimental::coroutine_handle<promise_type> m_coro;
};

namespace d{

	template<typename event_type>
	struct subscriber_thingy_impl {
		struct sentinal {};
		struct awaitable_iterator {
			awaitable_iterator(subscriber_thingy_impl& t) :parent(t) {}
			bool await_ready() {
				return false;
			}
			void await_suspend(std::experimental::coroutine_handle<> h) {
				parent.stuff.push({ h,&eventu });
			}
			awaitable_iterator await_resume() {
				return *this;
			}
			awaitable_iterator& operator++() { return *this; }
			awaitable_iterator& operator++(int) { return *this; }
			bool operator!=(sentinal)const { return true; }
			event_type& operator*() {
				return eventu;
			}
		private:
			event_type eventu;
			subscriber_thingy_impl& parent;
			friend subscriber_thingy_impl;
		};
		auto begin() {
			return awaitable_iterator{ *this };
		}
		auto end() { return sentinal{}; }
	protected:
		concurrent_queue<std::pair<std::experimental::coroutine_handle<>, event_type*>,std::vector<std::pair<std::experimental::coroutine_handle<>, event_type*>>> stuff;		
	};

	template<typename event_type>
	struct subscriber_thingy_async :subscriber_thingy_impl<event_type> {
		[[nodiscard]]
		std::future<void> add_event(event_type e) {
			return std::async(std::launch::async, [_e = std::move(e), this]()mutable {auto t = stuff.pop(); *t.second = std::move(_e); t.first.resume(); });
		}
	};

	template<typename event_type,typename Executor_t>
	struct subscriber_thingy2 :subscriber_thingy_impl<event_type> {
		subscriber_thingy2(Executor_t& t_exec):m_exec(t_exec){}

		void add_event(event_type e) {			
			m_exec.execute([_e =std::move(e),this](){
				auto t = this->stuff.pop();
				*t.second = std::move(e);
				t.first.resume();
			});
		}
		Executor_t& get_executor() noexcept{ return m_exec; }
	private:
		Executor_t& m_exec;
		
	};

	//template<typename event_type,typename Executor_t>	subscriber_thingy_no_async2(Executor_t&)->subscriber_thingy_no_async2<event_type, Executor_t>;

	template<typename event_type>
	struct subscriber_thingy_no_async :subscriber_thingy_impl<event_type> {
		void add_event(event_type e) {
			auto t = stuff.pop();
			*t.second = std::move(e);
			t.first.resume();
		}
	};

}

std::future<void> http_conn(d::subscriber_thingy_async<std::variant<discord_request, std::chrono::milliseconds>>& hand, client* parent);
*/