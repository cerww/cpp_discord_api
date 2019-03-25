#pragma once
#include "task.h"
#include <mutex>
//#include <boost/container/deque.hpp>

template<typename T>
struct concurrent_coro_queue{

	cerwy::task<T> pop() {
		std::lock_guard lock(m_mut);
		if(!m_data.empty()) {
			auto top = std::move(m_data.front());
			m_data.erase(m_data.begin());
			return cerwy::make_ready_task(std::move(top));
		}else {
			cerwy::promise<T> promise;
			auto task = promise.get_task();
			m_waiters.push_back(std::move(promise));
			return task;
		}
	}

	void push(T val) {
		std::lock_guard lock(m_mut);
		if(!m_waiters.empty()) {
			auto p = std::move(m_waiters.front());
			m_waiters.erase(m_waiters.begin());
			p.set_value(std::move(val));
		}else {
			m_data.push_back(std::move(val));
		}
	}

	void cancel_all() {
		std::vector<cerwy::promise<T>> waiters;
		{
			std::lock_guard lock(m_mut);
			std::swap(waiters, m_waiters);
		}
		for(auto& p:waiters) {			
			//p.set_exception(???);
		}
		
	}

	std::vector<T> pop_all() {
		std::lock_guard lock(m_mut);
		return std::move(m_data);
	}

private:
	std::mutex m_mut;
	std::vector<T> m_data;
	std::vector<cerwy::promise<T>> m_waiters;
};



