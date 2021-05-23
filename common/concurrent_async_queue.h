#pragma once
#include "eager_task.h"
#include <mutex>
//#include <boost/container/deque.hpp>
#include <span>
#include <algorithm>
#include "async_mutex.h"
#include <deque>
#include <iostream>

template<typename T>
struct concurrent_async_queue {

	cerwy::eager_task<T> pop() {
		std::lock_guard lock(m_mut);
		if (!m_data.empty()) {
			auto top = std::move(m_data.front());
			m_data.erase(m_data.begin());
			return cerwy::make_ready_task(std::move(top));
		} else {
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
		} else {
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
			} else {
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

	cerwy::eager_task<T> pop() {
		std::lock_guard lock(m_mut);
		if (!m_data.empty()) {
			auto top = std::move(m_data.front());
			m_data.erase(m_data.begin());
			return cerwy::make_ready_task(std::move(top));
		} else {
			cerwy::promise<T> promise;
			auto task = promise.get_task();
			m_waiter = std::move(promise);
			return task;
		}
	}

	std::optional<T> try_pop() {
		std::lock_guard lock(m_mut);
		if (m_data.empty()) {
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
		} else {
			m_data.push_back(std::move(val));
		}
	}

	void push_all(std::span<T> vals) {
		if (vals.empty()) {
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
			} else {
				m_data.insert(m_data.end(), vals.begin(), vals.end());
			}
		}
	}

	void cancel_all() {
		std::lock_guard lock(m_mut);
		if (m_waiter.has_value()) {
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

//use the waste 1 element when full method 
template<typename T, size_t ring_size = 16> //requires ring_size %2 == 0
struct ring_buffer_no_thread_stuffs {

	bool is_full() const noexcept {
		return (m_head + 1) % ring_size == m_tail;
	}

	bool is_empty() const noexcept {
		return m_head == m_tail;
	}

	bool try_push(T& thing) {
		if (is_full()) {
			return false;
		}
		m_data[m_tail++] = std::move(thing);
		m_tail %= ring_size;

		return true;
	}

	std::optional<T> try_pop() {
		if (is_empty()) {
			return std::nullopt;
		}
		T thing = std::move(m_data[m_head]);
		m_head = (m_head + 1) % ring_size;

		return thing;
	}

private:
	std::array<T, ring_size> m_data = {};
	int m_head = 0;
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
//
//
//

template<typename T>
struct async_queue_maybe_better {

	void push(T thing) {
		std::cout << (uint64_t)this << std::endl;
		auto coro_to_resume = [&]()->std::coroutine_handle<> {//iife for lock_guard
			std::lock_guard lock(m_mut);

			if (m_waiter) {
				awaiter* const waiter = std::exchange(m_waiter, m_waiter->next);
				waiter->thing = std::move(thing);
				return waiter->coro;
			}

			if (m_ring_buffer.try_push(thing)) {
				return nullptr;
			} else {
				m_overflow.push_back(std::move(thing));
				return nullptr;
			}
		}();

		if (coro_to_resume) {
			coro_to_resume.resume();
		}
	}

	struct awaiter {
		explicit awaiter(async_queue_maybe_better* a) :
			queue(a) {}

		bool await_ready() const noexcept {
			return false;
		}

		bool await_suspend(std::coroutine_handle<> t_coro) {
			std::lock_guard guard(queue->m_mut);
			coro = t_coro;
			if (!queue->is_empty()) {
				thing = queue->pop_1();
				return false;
			}
			next = queue->m_waiter;
			queue->m_waiter = this;
			return true;
		}

		T await_resume() {
			//return std::move(*thing);
			if(std::holds_alternative<T>(thing)) {
				return std::move(std::get<T>(thing));
			}else {
				throw std::get<std::exception_ptr>(thing);
			}
		}

		async_queue_maybe_better* queue = nullptr;
		awaiter* next = nullptr;
		std::variant<T,std::exception_ptr> thing;		
		std::coroutine_handle<> coro;
	};

	awaiter pop() {
		return awaiter(this);
	}

	void cancel_all() {
		auto waiters = [&]() {
			std::lock_guard lock(m_mut);
			return std::exchange(m_waiter, nullptr);
		}();

		while(waiters) {
			auto current_waiter = waiters;
			waiters = waiters->next;			
			current_waiter->thing = std::make_exception_ptr(1);
			current_waiter->coro.resume();
		}

	}

private:
	bool is_empty() const noexcept {
		return m_ring_buffer.is_empty();
	}
	
	//requires !is_empty()
	T pop_1() {
		auto a = m_ring_buffer.try_pop();
		if (!m_overflow.empty()) {
			auto next_thing_in_overflow = std::move(m_overflow.front());
			m_overflow.pop_front();
			m_ring_buffer.try_push(next_thing_in_overflow);
		}
		return std::move(*a);
	}

	std::mutex m_mut;
	ring_buffer_no_thread_stuffs<T, 32> m_ring_buffer;
	std::deque<T> m_overflow;
	awaiter* m_waiter = nullptr;
	friend awaiter;
};

template<typename T>
struct async_queue_thread_unsafe {

	struct awaiter {
		bool await_ready() {
			return !queue->data().empty();
		}

		void await_suspend(std::coroutine_handle<> coroutine_handle) {
			coro = coroutine_handle;
			next = std::exchange(queue->m_waiter_stack, this);
		}

		T await_resume() {
			if(exception.has_value()) {
				std::rethrow_exception(*exception);
			}
			if (next_in_queue) {
				return std::move(*next_in_queue);
			}
			else {
				auto ret = std::move(queue->data().front());
				queue->data().erase(queue->data().begin());
				return ret;
			}
		}

		async_queue_thread_unsafe* queue = nullptr;
		awaiter* next = nullptr;
		std::coroutine_handle<> coro;
		std::optional<T> next_in_queue;
		std::optional<std::exception_ptr> exception;
	};

	std::vector<T>& data() {
		return m_data;
	}

	const std::vector<T>& data()const {
		return m_data;
	}

	awaiter pop() {
		return awaiter{ this };
	}

	void push(T thing) {
		if (m_waiter_stack) {
			m_waiter_stack->next_in_queue = std::move(thing);
			auto me = std::exchange(m_waiter_stack, m_waiter_stack->next);
			me->coro.resume();
		}
		else {
			m_data.push_back(std::move(thing));
		}
	}

	void cancel_all() {
		auto stack = std::exchange(m_waiter_stack, nullptr);
		while(stack) {
			stack->exception = std::make_exception_ptr(1);
			std::exchange(stack, stack->next)->coro.resume();
		}
		
	}

	
private:
	friend awaiter;
	
	std::vector<T> m_data;
	awaiter* m_waiter_stack = nullptr;
};