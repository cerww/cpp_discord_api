#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <thread>
#include "requests.h"
//#include "concurrent_queue.h"
#include "../common/eager_task.h"
#include "../common/async_mutex.h"
#include "../common/concurrent_async_queue.h"
//#include <variant>

namespace cacheless {

struct discord_request {
	boost::beast::http::request<boost::beast::http::string_body> req;
	ref_count_ptr<rq::shared_state> state;
};

struct client;

struct http_connection {
	void sleep_till(std::chrono::system_clock::time_point time_point) {
		m_rate_limted_until = time_point;
		m_global_rate_limited.store(true);
	};

	http_connection() = delete;
	http_connection(http_connection&&) = delete;
	http_connection(const http_connection&) = delete;
	http_connection& operator=(const http_connection&) = delete;
	http_connection& operator=(http_connection&&) = delete;

	explicit http_connection(client*, boost::asio::io_context&);

	~http_connection() {
		m_done.store(true);
		m_request_queue.cancel();
		if (m_thread.joinable()) {
			m_thread.join();
		}
	}

	void send(discord_request&& d) {
		m_request_queue.push(d);
	}

	void stop() {
		m_done.store(true);
	}

	cerwy::eager_task<boost::beast::error_code> async_connect();

private:
	std::chrono::system_clock::time_point m_rate_limted_until = {};
	std::atomic<bool> m_global_rate_limited = false;

	std::atomic<bool> m_done = false;
	std::thread m_thread{};

	boost::asio::io_context& m_ioc;
	boost::asio::ip::tcp::resolver m_resolver;

	boost::asio::ssl::context m_sslCtx{boost::asio::ssl::context::tlsv12_client};
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_socket{m_ioc, m_sslCtx};
	boost::beast::flat_buffer m_buffer{};
	concurrent_queue<discord_request> m_request_queue = {};
	client* m_client = nullptr;

	void send_to_discord(discord_request);
	bool send_to_discord_(discord_request&, size_t);
	void connect();
	void reconnect();

	void send_rq(discord_request&);

	std::vector<std::tuple<size_t, std::chrono::system_clock::time_point, std::vector<discord_request>>> m_rate_limited_requests{};
};


/*
#include <coroutine>

struct coro_discord_http_connection{
	struct promise_type{
		std::suspend_never initial_suspend() {
			return {};
		}
		std::suspend_never final_suspend() {
			return {};
		}
		void unhandled_exception() {

		}

	};

	std::coroutine_handle<promise_type> m_coro;
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
			void await_suspend(std::coroutine_handle<> h) {
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
		concurrent_queue<std::pair<std::coroutine_handle<>, event_type*>,std::vector<std::pair<std::coroutine_handle<>, event_type*>>> stuff;
	};

	template<typename event_type>
	struct subscriber_thingy_async :subscriber_thingy_impl<event_type> {
		[[nodiscard]]
		std::future<void> add_event(event_type e) {
			return std::async([_e = std::move(e), this]()mutable {auto t = stuff.pop(); *t.second = std::move(_e); t.first.resume(); });
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


struct http_connection2 {
	void sleep_till(std::chrono::system_clock::time_point time_point) {
		m_rate_limted_until = time_point;
		m_global_rate_limited.store(true);
	};

	http_connection2() = delete;
	http_connection2(http_connection2&&) = delete;
	http_connection2(const http_connection2&) = delete;
	http_connection2& operator=(const http_connection2&) = delete;
	http_connection2& operator=(http_connection&&) = delete;

	explicit http_connection2(client*, boost::asio::io_context&);

	~http_connection2() {
		m_done.store(true);
		m_request_queue.cancel_all();
		if (m_thread.joinable())
			m_thread.join();
	}

	void send(discord_request&& d);

	void stop() {
		m_done.store(true);
	}

	cerwy::eager_task<boost::beast::error_code> async_connect();

private:
	std::chrono::system_clock::time_point m_rate_limted_until = {};
	std::atomic<bool> m_global_rate_limited = false;

	std::atomic<bool> m_done = false;
	std::thread m_thread{};

	boost::asio::io_context& m_ioc;
	boost::asio::ip::tcp::resolver m_resolver;
	async_mutex m_mut;

	boost::asio::ssl::context m_ssl_ctx{boost::asio::ssl::context::tlsv12_client};
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_socket{m_ioc, m_ssl_ctx};


	boost::beast::flat_buffer m_buffer{};
	mpsc_concurrent_async_queue<discord_request> m_request_queue = {};
	client* m_client = nullptr;

	cerwy::eager_task<void> send_to_discord(discord_request);
	cerwy::eager_task<void> send_to_discord(discord_request, uint64_t major_param_id_);

	cerwy::eager_task<bool> send_to_discord_(discord_request&);

	cerwy::eager_task<void> start_sending();

	void resend_rate_limted_requests_for(uint64_t);
	void rate_limit_id(uint64_t major_param_id_, std::chrono::system_clock::time_point, std::optional<discord_request>);
	bool check_rate_limit(uint64_t id, discord_request& rq);

	//void connect();
	cerwy::eager_task<void> reconnect();

	cerwy::eager_task<void> send_rq(discord_request&);


	//sorted by order of coming off the queue
	//not a queue/deque since it's size shuold be small
	//can be a priority queue?
	//can't be priority queue since i need to look at things inside
	//Q: sort by id or time?

	struct rate_limit_entry {
		uint64_t id;
		std::chrono::system_clock::time_point until;
		std::vector<discord_request> requests;
	};

	std::vector<rate_limit_entry> m_rate_limited_requests{};
	std::mutex m_rate_limit_mut;


};

}
