#pragma once
#include "task.h"
#include <mutex>
#include "concurrent_queue.h"

struct get_current_coroutine_handle{
	static bool await_ready() {
		return false;
	}

	void await_suspend(std::experimental::coroutine_handle<> h) {
		me = h;
		me.resume();
	}

	std::experimental::coroutine_handle<> await_resume() const {
		return me;
	}

	std::experimental::coroutine_handle<> me;
};

struct resume_coro{
	//resume_coro
	cerwy::task<void> pause() {
		auto current_coro = co_await get_current_coroutine_handle();
		co_await std::experimental::suspend_always();
	}

	void resume() {
		auto t = std::exchange(current_waiter,nullptr);
		t.resume();		
	}

	std::experimental::coroutine_handle<> current_waiter = nullptr;
};

struct async_mutex{
	struct lock_t{
		lock_t() = default;
		explicit lock_t(async_mutex* m):m_mut(m){}
		lock_t(const lock_t&) = delete;

		lock_t(lock_t&& o) noexcept :m_mut(std::exchange(o.m_mut, nullptr)) {}

		lock_t& operator=(const lock_t&) = delete;

		lock_t& operator=(lock_t&& c) noexcept {
			lock_t temp(std::move(c));
			std::swap(temp.m_mut, m_mut);
			return *this;
		};

		~lock_t() {
			if(m_mut)
				m_mut->unlock();
		}
	private:
		async_mutex* m_mut = nullptr;
	};

	
	void unlock() {
		if(!m_waiters.empty()) {
			if (m_unlocker) {
				m_unlocker.resume();
			}else {
				unlock_impl();
			}
		}			
	}

	cerwy::task<lock_t> lock() {
		if(!m_is_locked) {
			m_is_locked = true;
			return cerwy::make_ready_task(lock_t(this));			
		} else {
			cerwy::promise<lock_t> promise;
			auto ret = promise.get_task();
			m_waiters.push_back(std::move(promise));
			return ret;
		}
	}

private:
	cerwy::task<void> unlock_impl() {
		m_unlocker = co_await get_current_coroutine_handle();

		{//do once without the suspend
			auto t = std::move(m_waiters.front());
			m_waiters.erase(m_waiters.begin());
			t.set_value(lock_t(this));
		}
		while(!m_waiters.empty()) {
			co_await std::experimental::suspend_always();
			auto t = std::move(m_waiters.front());
			m_waiters.erase(m_waiters.begin());
			t.set_value(lock_t(this));
		}

		m_unlocker = nullptr;
		m_is_locked = false;
	}
	std::experimental::coroutine_handle<> m_unlocker = nullptr;
	std::vector<cerwy::promise<lock_t>> m_waiters;
	bool m_is_locked = false;
};

struct async_mutex2{
	struct lock_t {
		lock_t() = default;
		explicit lock_t(async_mutex2* m) :m_mut(m) {}
		lock_t(const lock_t&) = delete;

		lock_t(lock_t&& o) noexcept :m_mut(std::exchange(o.m_mut, nullptr)) {}

		lock_t& operator=(const lock_t&) = delete;

		lock_t& operator=(lock_t&& c) noexcept {
			lock_t temp(std::move(c));
			std::swap(temp.m_mut, m_mut);
			return *this;
		};

		~lock_t() {
			if (m_mut)
				m_mut->unlock();
		}
	private:
		async_mutex2* m_mut = nullptr;
	};

	void unlock() {		
		if(!m_waiters.empty()) {
			if (m_count_state & 3) {
				auto t = std::move(m_waiters.front());
				m_waiters.erase(m_waiters.begin());
				t.set_value(lock_t(this));
			}else {
				auto t = std::move(m_waiters.back());
				m_waiters.pop_back();
				t.set_value(lock_t(this));
			}
		}else {
			m_is_locked = false;
		}
		++m_count_state;
	}

	cerwy::task<lock_t> lock() {
		if (!m_is_locked) {
			m_is_locked = true;
			return cerwy::make_ready_task(lock_t(this));
		} else {
			cerwy::promise<lock_t> promise;
			auto ret = promise.get_task();
			m_waiters.push_back(std::move(promise));
			return ret;
		}
	}

private:
	size_t m_count_state = 0;
	std::vector<cerwy::promise<lock_t>> m_waiters;
	bool m_is_locked = false;
};

//TODO make this work
struct async_concurrent_mutex{
	struct lock_t {
		lock_t() = default;
		explicit lock_t(async_concurrent_mutex* m) :m_mut(m) {}
		lock_t(const lock_t&) = delete;

		lock_t(lock_t&& o) noexcept :m_mut(std::exchange(o.m_mut, nullptr)) {}

		lock_t& operator=(const lock_t&) = delete;

		lock_t& operator=(lock_t&& c) noexcept {
			lock_t temp(std::move(c));
			std::swap(temp.m_mut, m_mut);
			return *this;
		};

		~lock_t() {
			if (m_mut)
				m_mut->unlock();
		}
	private:
		async_concurrent_mutex* m_mut = nullptr;
	};

	cerwy::task<lock_t> lock() {
		if (m_is_locked.exchange(true)){
			cerwy::promise<lock_t> p;
			auto ret = p.get_task();
			m_queue.push(std::move(p));
			return ret;
		}else {
			return cerwy::make_ready_task(lock_t(this));
		}
	}

	void unlock() {
		auto next = m_queue.try_pop();
		if(next) {
			next->set_value(lock_t(this));
		}else {
			m_is_locked = false;
			if (next = m_queue.try_pop();next) {//in case another thread calls lock just before m_is_locked = false
				next->set_value(lock_t(this));
			}
		}
	}

private:
	std::atomic<bool> m_is_locked = false;
	concurrent_queue<cerwy::promise<lock_t>> m_queue;
};

/*

cerwy::task<void> t1(async_mutex2& mut,cerwy::task<int> tasky) {
	auto t = co_await mut.lock();
	std::cout << "a" << std::endl;;
	auto y = co_await tasky;
	std::cout << "b" << std::endl;;
}

cerwy::task<void> t2(async_mutex2& mut) {
	auto t = co_await mut.lock();
	std::cout << "c" << std::endl;;
}

int main(){
	async_mutex2 mut;
	cerwy::promise<int> promise;

	t1(mut,promise.get_task());
	t2(mut);
	promise.set_value(1);

	cerwy::promise<int> promise2;
	t1(mut, promise2.get_task());
	t2(mut);
	promise2.set_value(1);

	std::cin.get();
}
*/