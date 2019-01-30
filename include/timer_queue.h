#pragma once
#include <vector>
#include <mutex>
#include <utility>
#include <optional>
#include <memory>
#include <algorithm>
#include <queue>

template<typename comp>
struct compare_second {
	template<typename T1, typename T2, typename U1, typename U2>
	constexpr bool operator()(const std::pair<T1, T2>& a, const std::pair<U1, U2>& b) const noexcept(noexcept(comp{}(a.second, b.second))) {
		return comp{}(a.second, b.second);
	}
};

template<typename T, typename time_point = std::chrono::steady_clock::time_point, typename A = std::allocator<T>>
struct timer_queue {
	using allocator_type = typename std::allocator_traits<A>::template rebind_alloc<std::pair<T, time_point>>;
	using clock_t = typename time_point::clock;

	timer_queue() = default;
	explicit timer_queue(A a) :m_queue(container(std::move(a))) {}

	void push(T a, std::chrono::steady_clock::time_point tp) {
		m_queue.emplace(std::move(a), tp);
	}

	template<typename duration>
	void push(T a, duration&& d) {
		push(std::move(a), clock_t::now() + d);
	}

	std::optional<T> try_pop() noexcept {
		if (m_queue.empty() || m_queue.top().second > clock_t::now())
			return std::nullopt;
		return do_pop();
	}

	std::pair<std::optional<T>, std::optional<time_point>> try_pop_next_time() {
		if (m_queue.empty())
			return { std::nullopt ,std::nullopt };
		if (m_queue.top().second < clock_t::now()) {
			auto r = do_pop();
			const auto next_tp = time_till_next();
			return { std::move(r),next_tp };
		}
		return { std::nullopt,time_till_next() };
	}

	std::optional<time_point> time_till_next()const noexcept {
		if (m_queue.empty())
			return std::nullopt;
		return m_queue.top().second;
	}

	size_t size()const noexcept {
		return m_queue.size();
	}

private:
	T do_pop() {
		//[[assert: !m_queue.empty()]];
		auto r = std::move(const_cast<T&>(m_queue.top().first));//;-;
		m_queue.pop();
		return r;
	}

	std::priority_queue<std::pair<T, time_point>, std::vector<std::pair<T, time_point>, allocator_type>, compare_second<std::greater<>>> m_queue;
};

template<typename T, typename time_point = std::chrono::steady_clock::time_point, typename A = std::allocator<T>>
struct concurrent_timer_queue {
	using allocator_type = typename std::allocator_traits<A>::template rebind_alloc<std::pair<T, time_point>>;
	using clock_t = typename time_point::clock;
	concurrent_timer_queue() = default;

	explicit concurrent_timer_queue(A a) :m_queue(std::move(a)) {}

	void push(T a, time_point tp) {
		{
			std::unique_lock locky(m_mut);
			m_queue.push(std::move(a), tp);
		}
		m_cv.notify_one();
	}

	template<typename duration>
	void push(T a, duration&& d) {
		{
			std::unique_lock locky(m_mut);
			m_queue.push(std::move(a), std::forward<duration>(d) + clock_t::now());
		}
		m_cv.notify_one();
	}

	/*
	T pop() {
		std::unique_lock lock(m_mut);
		m_cv.wait(lock,[this](){
			return m_queue.size() && ;
		});

	}
	*/
	std::optional<T> try_pop() noexcept {
		std::lock_guard locky(m_mut);
		if (m_queue.empty() || m_queue[0].second > clock_t::now())
			return std::nullopt;
		return unsafe_pop();
	}

	std::optional<T> try_pop_busy() noexcept {
		std::unique_lock locky(m_mut, std::try_to_lock);
		if (!locky || m_queue.empty() || m_queue[0].second > clock_t::now())
			return std::nullopt;
		return unsafe_pop();
	}

	std::pair<std::optional<T>, std::optional<time_point>> try_pop_next_time() {
		std::lock_guard lock(m_mut);
		return m_queue.try_pop_next_time();
	}

	std::pair<std::optional<T>, std::optional<time_point>> try_pop_next_time_blocking() {
		std::unique_lock lock(m_mut);
		m_cv.wait(lock, [this](){
			return m_queue.size();
		});
		return m_queue.try_pop_next_time();		
	}

	std::pair<std::optional<T>, std::optional<time_point>> pop_next_time_busy() {
		std::unique_lock locky(m_mut, std::try_to_lock);
		if (!locky)
			return { std::nullopt,std::nullopt };
		return m_queue.try_pop_next_time();
	}

	std::optional<time_point> time_till_next()const noexcept {
		std::lock_guard locky(m_mut);
		return m_queue.time_till_next();
	}

	std::optional<time_point> time_till_next_busy()const noexcept {
		std::unique_lock locky(m_mut, std::try_to_lock);
		if (!locky)
			return std::nullopt;
		return m_queue.time_till_next();
	}

	std::optional<time_point> time_till_next_unsafe()const noexcept {
		return m_queue.time_till_next();
	}

	void reserve(size_t n) {
		std::lock_guard locky(m_mut);
		m_queue.reserve(n);
	}

private:
	T unsafe_pop() {
		return m_queue.do_pop();
	}

	timer_queue<T, time_point, allocator_type> m_queue;
	std::mutex m_mut;
	std::condition_variable m_cv;
};


