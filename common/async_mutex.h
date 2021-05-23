#pragma once
#include "eager_task.h"
#include <mutex>
#include "concurrent_queue.h"

struct get_current_coroutine_handle {
	static bool await_ready() {
		return false;
	}

	void await_suspend(std::coroutine_handle<> h) {
		me = h;
		me.resume();
	}

	std::coroutine_handle<> await_resume() const {
		return me;
	}

	std::coroutine_handle<> me;
};

struct resume_coro {
	//resume_coro
	cerwy::eager_task<void> pause() {
		auto current_coro = co_await get_current_coroutine_handle();
		co_await std::suspend_always();
	}

	void resume() {
		auto t = std::exchange(current_waiter, nullptr);
		t.resume();
	}

	std::coroutine_handle<> current_waiter = nullptr;
};

struct async_mutex1 {
	struct lock_t {
		lock_t() = default;

		explicit lock_t(async_mutex1* m):
			m_mut(m) {}

		lock_t(const lock_t&) = delete;

		lock_t(lock_t&& o) noexcept :
			m_mut(std::exchange(o.m_mut, nullptr)) {}

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
		async_mutex1* m_mut = nullptr;
	};


	void unlock() {
		if (!m_waiters.empty()) {
			if (m_unlocker) {
				m_unlocker.resume();
			}
			else {
				unlock_impl();
			}
		}
	}

	cerwy::eager_task<lock_t> lock() {
		if (!m_is_locked) {
			m_is_locked = true;
			return cerwy::make_ready_task(lock_t(this));
		}
		else {
			cerwy::promise<lock_t> promise;
			auto ret = promise.get_task();
			m_waiters.push_back(std::move(promise));
			return ret;
		}
	}

private:
	cerwy::eager_task<void> unlock_impl() {
		m_unlocker = co_await get_current_coroutine_handle();

		{//do once without the suspend
			auto t = std::move(m_waiters.front());
			m_waiters.erase(m_waiters.begin());
			t.set_value(lock_t(this));
		}
		while (!m_waiters.empty()) {
			co_await std::suspend_always();
			auto t = std::move(m_waiters.front());
			m_waiters.erase(m_waiters.begin());
			t.set_value(lock_t(this));
		}

		m_unlocker = nullptr;
		m_is_locked = false;
	}

	std::coroutine_handle<> m_unlocker = nullptr;
	std::vector<cerwy::promise<lock_t>> m_waiters;
	bool m_is_locked = false;
};

struct async_mutex2 {
	struct lock_t {
		lock_t() = default;

		explicit lock_t(async_mutex2* m) :
			m_mut(m) {}

		lock_t(const lock_t&) = delete;

		lock_t(lock_t&& o) noexcept :
			m_mut(std::exchange(o.m_mut, nullptr)) {}

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
		if (!m_waiters.empty()) {
			if (m_count_state & 3) {
				auto t = std::move(m_waiters.front());
				m_waiters.erase(m_waiters.begin());
				t.set_value(lock_t(this));
			}
			else {
				auto t = std::move(m_waiters.back());
				m_waiters.pop_back();
				t.set_value(lock_t(this));
			}
		}
		else {
			m_is_locked = false;
		}
		++m_count_state;
	}

	cerwy::eager_task<lock_t> lock() {
		if (!m_is_locked) {
			m_is_locked = true;
			return cerwy::make_ready_task(lock_t(this));
		}
		else {
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


struct async_mutex {
	struct async_lock_t {
		explicit async_lock_t() = default;

		explicit async_lock_t(async_mutex* p) :
			m_parent(p) {}

		async_lock_t(async_lock_t&& o) noexcept :
			m_parent(std::exchange(o.m_parent, nullptr)) {}

		async_lock_t& operator=(async_lock_t&& o) noexcept {
			std::swap(o.m_parent, m_parent);
			return *this;
		}

		async_lock_t(const async_lock_t&) = delete;
		async_lock_t& operator=(const async_lock_t&) = delete;

		~async_lock_t() {
			if (m_parent)
				m_parent->unlock();
		}

	private:
		async_mutex* m_parent = nullptr;
	};

	struct awaiter_for_lock {
		awaiter_for_lock() = default;

		bool await_ready() {
			return false;
		}

		bool await_suspend(std::coroutine_handle<> h) {
			m_handle = h;
			auto old_state = m_parent->try_lock(this);
			if (old_state == unlocked) {
				//h.resume();
				return false;
			}
			else {
				//m_next = (decltype(this))old_state;
				//return true;
				//assert(old_state == (uintptr_t)m_next);
				return true;
			}
		}

		async_lock_t await_resume() {
			return async_lock_t(m_parent);
		}


	private:
		friend async_mutex;
		//private so this can only be created from .lock()
		explicit awaiter_for_lock(async_mutex* p) :
			m_parent(p) {}

		async_mutex* m_parent = nullptr;
		std::coroutine_handle<> m_handle = nullptr;
		awaiter_for_lock* m_next = nullptr;

	};

	auto async_lock() {
		return awaiter_for_lock(this);
	}

	void unlock() {
		assert(m_state != unlocked);

		if (!m_waiters) {
			//m_state == locked_no_waiters or ptr
			auto next = locked_no_waiters;
			if (m_state.compare_exchange_strong(next, unlocked)) {
				return;
			}

			m_waiters = (awaiter_for_lock*)m_state.exchange(locked_no_waiters, std::memory_order_acquire);

		}
		assert(m_waiters);
		auto next = std::exchange(m_waiters, m_waiters->m_next);
		next->m_handle.resume();
	}

	bool try_lock() {
		auto is_unlocked = unlocked;
		return m_state.compare_exchange_strong(is_unlocked, locked_no_waiters);
	}

	bool is_locked() const {
		return m_state.load() != unlocked;
	}


	bool is_unlocked() const {
		return m_state.load() == unlocked;
	}

private:
	friend awaiter_for_lock;
	/*
	same as the following, but atomic:

	if(m_state == unlocked){
		m_state = locked_no_waiters;
		return unlocked;
	}else{
		return m_state.exchange(ptr);
	}
	*/
	uintptr_t try_lock(awaiter_for_lock* ptr_) {
		auto ptr = (uintptr_t)ptr_;

		auto old_state = m_state.load();
		while (true) {
			if (old_state == unlocked) {
				if (m_state.compare_exchange_strong(old_state, locked_no_waiters)) {
					return unlocked;
				}
			}
			else {
				ptr_->m_next = (awaiter_for_lock*)old_state;
				if (m_state.compare_exchange_strong(old_state, ptr)) {
					return old_state;
				}
			}
		}

	}

	static constexpr uintptr_t unlocked = 1;

	static constexpr uintptr_t locked_no_waiters = 0;

	std::atomic<uintptr_t> m_state = unlocked;

	awaiter_for_lock* m_waiters = nullptr;
};

/*

cerwy::eager_task<void> t1(async_mutex2& mut,cerwy::eager_task<int> tasky) {
	auto t = co_await mut.lock();
	std::cout << "a" << std::endl;;
	auto y = co_await tasky;
	std::cout << "b" << std::endl;;
}

cerwy::eager_task<void> t2(async_mutex2& mut) {
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
