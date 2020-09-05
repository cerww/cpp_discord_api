#pragma once
#include "task.h"
#include <mutex>
//#include <boost/container/deque.hpp>
#include <span>
#include <algorithm>

template<typename T>
struct concurrent_async_queue {

	cerwy::task<T> pop() {
		std::lock_guard lock(m_mut);
		if (!m_data.empty()) {
			auto top = std::move(m_data.front());
			m_data.erase(m_data.begin());
			return cerwy::make_ready_task(std::move(top));
		}
		else {
			cerwy::promise<T> promise;
			auto task = promise.get_task();
			m_waiters.push_back(std::move(promise));
			return task;
		}
	}

	void push(T val) {
		std::unique_lock lock(m_mut);
		
		if (!m_waiters.empty()) {			
			auto p = std::move(m_waiters.front());
			m_waiters.erase(m_waiters.begin());
			lock.unlock();
			p.set_value(std::move(val));
		}
		else {
			m_data.push_back(std::move(val));
		}
	}

	void push_all(std::span<T> vals) {
		{
			std::unique_lock lock(m_mut);
			if (!m_waiters.empty()) {
				//m_data is empty
				//this can be done with zip
				const auto n = std::min(vals.size(), m_waiters.size());				
				std::vector<cerwy::promise<T>> waiters_to_resume(n);	
				std::move(m_waiters.begin(), m_waiters.begin() + n, waiters_to_resume.begin());				
				m_waiters.erase(m_waiters.begin(), m_waiters.begin() + n);				
				m_data.insert(m_data.end(), vals.begin() + n, vals.end());
				lock.unlock();
			}
			else {
				m_data.insert(m_data.end(), vals.begin(), vals.end());
			}
		}
	}

	void cancel_all() {
		std::vector<cerwy::promise<T>> waiters;
		{
			std::lock_guard lock(m_mut);
			std::swap(waiters, m_waiters);
		}
		for (auto& p : waiters) {
			p.set_exception(std::make_exception_ptr(1));
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

	//has waiter => no data
};

//give access to items somehow?
template<typename T>
struct mpsc_concurrent_async_queue {

	cerwy::task<T> pop() {
		std::lock_guard lock(m_mut);
		if (!m_data.empty()) {
			auto top = std::move(m_data.front());
			m_data.erase(m_data.begin());
			return cerwy::make_ready_task(std::move(top));
		}
		else {
			cerwy::promise<T> promise;
			auto task = promise.get_task();
			m_waiter = std::move(promise);
			return task;
		}
	}

	std::optional<T> try_pop() {
		std::lock_guard lock(m_mut);
		if(m_data.empty()) {
			return std::nullopt;
		}
		auto first = std::move(m_data.front());
		m_data.erase(m_data.begin());
		return first;
	}

	void push(T val) {
		std::unique_lock lock(m_mut);

		if (m_waiter.has_value()) {
			auto p = std::move(m_waiter.value());
			m_waiter = std::nullopt;
			lock.unlock();
			p.set_value(std::move(val));
		}
		else {
			m_data.push_back(std::move(val));
		}
	}

	void push_all(std::span<T> vals) {
		if(vals.empty()) {
			return;
		}
		{
			std::unique_lock lock(m_mut);
			if (m_waiter.has_value()) {
				//data is empty
				auto p = std::move(m_waiter.value());
				m_waiter = std::nullopt;
				
				m_data.resize(vals.size());
				std::move(vals.begin() + 1, vals.end(), m_data.begin());
				lock.unlock();
				p.set_value(std::move(vals.front()));
			}
			else {
				m_data.insert(m_data.end(), vals.begin(), vals.end());
			}
		}
	}

	void cancel_all() {
		std::lock_guard lock(m_mut);
		if(m_waiter.has_value()) {
			m_waiter->set_exception(std::make_exception_ptr(1));
		}
		
	}

	std::vector<T> pop_all() {
		std::lock_guard lock(m_mut);
		return std::move(m_data);
	}

	std::vector<T> get_data_copy() {
		std::lock_guard lock(m_mut);
		return m_data;
	}

	std::optional<T> pop_idx(size_t idx) {
		std::lock_guard lock(m_mut);
		if (idx >= m_data.size()) {
			return std::nullopt;
		}
		auto thing = std::move(m_data[idx]);
		m_data.erase(m_data.begin() + idx);
		return thing;		
	}

private:
	std::mutex m_mut;
	std::vector<T> m_data;
	std::optional<cerwy::promise<T>> m_waiter;

	//has waiter => no data
};

template<typename T,size_t ring_size = 16> //requires ring_size %2 == 0
struct async_ring_buffer_queue {

	bool is_full()const noexcept {
		return m_head == m_tail;
	}

	bool is_empty()const noexcept {
		return (m_head + 1) % ring_size == m_tail;
	}
private:
	std::array<T, ring_size> m_data;
	int m_head = ring_size-1;
	int m_tail = 0;
};
//size = 8
//________ tail = 0, head = 7
//aaaaaaaa tail = s - 1, head = s - 1
//________ tail = 0, head = s-1 empty
//________ tail = 1, head = 0 head+1 == tail
//________ tail = 5, head = 4 empty
//________ tail = 4, head = 4 full
//
//
//_aaa___
//es_____ full
//s=e empty?