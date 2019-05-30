#pragma once
#include <utility>
#include <boost/asio/async_result.hpp>
#include <tuple>

namespace cerwy {
	template<typename E, typename T>
	struct executor_binder {
		executor_binder(E& executor, T handler) :
			m_executor(&executor), 
			m_handler(std::move(handler)) {}

		T& handler() {
			return m_handler;
		}

		E& executor() {
			return *m_executor;
		}

	private:
		E* m_executor;
		T m_handler;
	};

	template<typename H, typename E>
	executor_binder<E,H> bind_executor(E& executor,H handler) {
		return executor_binder<E, H>(executor, std::move(handler));
	}
}

namespace boost::asio{
	template<typename E,typename T,typename...sig>
	class async_result<cerwy::executor_binder<E, T>, void(sig...)> {
	public:
		using return_type = typename async_result<T, void(sig...)>::return_type;
		using underlying_handler_type = typename async_result<T, void(sig...)>::completion_handler_type;

		struct completion_handler_type{
			explicit completion_handler_type(cerwy::executor_binder<E,T> rawr):
				m_handler(rawr.handler()),
				m_executor(&rawr.executor()){				
			}

			underlying_handler_type& underlying_handler() {
				return m_handler;
			}

			template<typename ...Args>
			void operator()(Args&&... args) {				
				m_executor->post([stuff = std::make_tuple(std::forward<Args>(args)...),handler = std::move(m_handler)]()mutable {
					std::apply(handler, std::move(stuff));
				});
			}

			underlying_handler_type m_handler;
			E* m_executor;
		};

		explicit async_result(completion_handler_type& h):
			m_result(h.underlying_handler()){}

		return_type get() {
			return m_result.get();
		}

	private:
		async_result<T, void(sig...)> m_result;
	};
}

