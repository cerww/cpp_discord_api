#pragma once
#include "eager_task.h"
#include <boost/asio/async_result.hpp>
#include <boost/system/system_error.hpp>

struct use_task_t { };

struct use_task_return_ec_t {};

static constexpr inline use_task_return_ec_t use_task_return_ec;

static constexpr inline use_task_t use_task;

template<typename fn>
struct packaged_task_t {
	fn f;
};

template<typename fn>
packaged_task_t<fn> use_packaged_task(fn&& f) {
	return packaged_task_t<fn>{std::forward<fn>(f)};
};

struct args_to_tuple_t {
	template<typename... args>
	constexpr auto operator()(args&&... Args) const noexcept(std::make_tuple(std::forward<args>(Args)...))
	->decltype(std::make_tuple(std::forward<args>(Args)...)) {
		
		return std::make_tuple(std::forward<args>(Args)...);
	}
};

struct use_task_return_tuple2_t {};

static constexpr inline const use_task_return_tuple2_t use_task_return_tuple2;

static constexpr inline const args_to_tuple_t args_to_tuple;

static inline constexpr const packaged_task_t<args_to_tuple_t> use_task_return_tuple;

template<typename ...sig>
struct use_task_return_tuple3_t {
	use_task_return_tuple3_t() = default;

	cerwy::eager_task<std::tuple<sig...>> get_task() {
		return m_promise.get_task();
	}

	void operator()(sig ... vals) {
		m_promise.set_value(std::make_tuple(std::move(vals)...));
	}

private:
	cerwy::promise<std::tuple<sig...>> m_promise;
};

namespace boost::asio {
	template<typename R>
	class async_result<use_task_t, void(system::error_code, R)> {
	public:
		using return_type = cerwy::eager_task<R>;

		struct completion_handler_type {
			explicit completion_handler_type(use_task_t) {}

			void operator()(const system::error_code& ec, R n) {
				if (ec) promise.set_exception(std::make_exception_ptr(system::system_error(ec)));
				else promise.set_value(std::move(n));
			}

			cerwy::promise<R> promise;
		};

		explicit async_result(completion_handler_type& h):
			m_return_obj(h.promise.get_task()) {}

		return_type get() { return std::move(m_return_obj); }

	private:
		cerwy::eager_task<R> m_return_obj;
	};

	template<>
	class async_result<use_task_t, void(system::error_code)> {
	public:
		using return_type = cerwy::eager_task<void>;

		struct completion_handler_type {
			explicit completion_handler_type(use_task_t) {}

			void operator()(const system::error_code& ec) {
				if (ec)
					promise.set_exception(std::make_exception_ptr(system::system_error(ec)));
				else 
					promise.set_value();
			}

			cerwy::promise<void> promise{};
		};

		explicit async_result(completion_handler_type& h) :
			m_return_obj(h.promise.get_task()) {}

		return_type get() { return std::move(m_return_obj); }

	private:
		cerwy::eager_task<void> m_return_obj;
	};

	template<>
	class async_result<use_task_return_ec_t, void(system::error_code)> {
	public:
		using return_type = cerwy::eager_task<system::error_code>;

		struct completion_handler_type {
			explicit completion_handler_type(use_task_return_ec_t) {}

			void operator()(system::error_code ec) {
				promise.set_value(ec);
			}

			cerwy::promise<system::error_code> promise{};
		};

		explicit async_result(completion_handler_type& h) :
			m_return_obj(h.promise.get_task()) {}

		return_type get() { return std::move(m_return_obj); }

	private:
		cerwy::eager_task<system::error_code> m_return_obj;
	};

	template<typename fn, typename... sig_args>
	class async_result<packaged_task_t<fn>, void(sig_args ...)> {
	public:
		using task_return_type = std::invoke_result_t<fn, sig_args...>;
		using return_type = cerwy::eager_task<task_return_type>;

		struct completion_handler_type {
			explicit completion_handler_type(packaged_task_t<fn> pa):
				f(std::move(pa.f)) {}

			template<typename ...args>
			void operator()(args&&... stuff) {
				if constexpr (!std::is_void_v<task_return_type>)
					promise.set_value(std::invoke(f, std::forward<args>(stuff)...));
				else {
					std::invoke(f, std::forward<args>(stuff)...);
					promise.set_value();
				}
			}

			cerwy::promise<task_return_type> promise{};
			fn f;
		};

		explicit async_result(completion_handler_type& h) :
			m_return_obj(h.promise.get_task()) {}

		return_type get() { return std::move(m_return_obj); }

	private:
		cerwy::eager_task<task_return_type> m_return_obj;
	};

	template<typename... sig_args>
	class async_result<use_task_return_tuple2_t, void(sig_args ...)> {
	public:
		using task_return_type = std::tuple<std::decay_t<sig_args>...>;

		using return_type = cerwy::eager_task<task_return_type>;

		struct completion_handler_type {
			explicit completion_handler_type(use_task_return_tuple2_t pa) {}

			template<typename ...args>
			void operator()(args&&... stuff) {
				promise.set_value(std::make_tuple(std::forward<args>(stuff)...));
			}

			cerwy::promise<task_return_type> promise{};
			/*
			explicit operator async_result()const {
				return async_result(promise.get_task());
			}
			*/
		};

		explicit async_result(completion_handler_type& h) :
			m_return_obj(h.promise.get_task()) {}

		explicit async_result(completion_handler_type&& h) :
			m_return_obj(h.promise.get_task()) {}

		return_type get() { return std::move(m_return_obj); }

	private:
		//async_result(const async_result&) = delete;
		cerwy::eager_task<task_return_type> m_return_obj;
	};

	template<typename...sig, typename ...realsig>
	class async_result<use_task_return_tuple3_t<sig...>, void(realsig ...)> {
	public:
		using return_type = cerwy::eager_task<std::tuple<sig...>>;
		using completion_handler_type = use_task_return_tuple3_t<sig...>;

		async_result(completion_handler_type& c):
			m_ret_obj(c.get_task()) {}

		return_type get() {
			return std::move(m_ret_obj);
		}

	private:
		return_type m_ret_obj;
	};

}
