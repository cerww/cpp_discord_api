#pragma once
#include <boost/container/deque.hpp>
#include <condition_variable>

template<typename U, typename = void>
struct has_pop_front :std::false_type {};

template<typename U>
struct has_pop_front<U, std::void_t<decltype(std::declval<U>().pop_front())>> :std::true_type {};

template<typename T, typename container = boost::container::deque<T>>
struct concurrent_queue {
	void push(T t) {
		{
			std::lock_guard<std::mutex> locky(m_mut);
			m_data.push_back(std::move(t));
		}
		m_cv.notify_one();
	}

	template<typename... args>
	void emplace(args&&...Args) {
		{
			std::lock_guard<std::mutex> locky(m_mut);
			m_data.emplace_back(std::forward<args>(Args)...);
		}
		m_cv.notify_one();
	}

	T pop() {
		std::unique_lock<std::mutex> locky(m_mut);
		m_cv.wait(locky, [&]() {
			return m_data.size() ||
					m_is_canceled.load(std::memory_order_relaxed);
		});
		maybe_stop();
		return do_pop();
	}

	std::optional<T> try_pop_busy() {
		std::unique_lock<std::mutex> locky(m_mut, std::try_to_lock);
		if (!locky || m_data.empty()) 
			return std::nullopt;
		return std::optional<T>(do_pop());
	}

	std::optional<T> try_pop() {
		std::unique_lock<std::mutex> locky(m_mut);
		if (m_data.empty())
			return std::nullopt;
		return std::optional<T>(do_pop());
	}

	//false means that now >= time_point
	template<typename time_point>
	std::optional<T> try_pop_until(time_point end) {
		if (time_point::clock::now() > end)
			return std::nullopt;
		std::unique_lock<std::mutex> locky(m_mut);
		if (m_cv.wait_until(locky, end, [this]() { return !m_data.empty(); }))
			return do_pop();
		return std::nullopt;
	}

	template<typename duration>
	std::optional<T> try_pop_for(duration d) {
		std::unique_lock<std::mutex> locky(m_mut);
		if (m_cv.wait_for(locky, d, [this]() { return !m_data.empty(); }))
			return do_pop();
		return std::nullopt;
	}

	size_t size() const noexcept {
		return m_data.size();
	}

	auto begin() noexcept {
		return m_data.begin();
	}

	auto begin() const noexcept {
		return m_data.begin();
	}

	auto end() const noexcept {
		return m_data.end();
	}

	auto end() noexcept {
		return m_data.end();
	}

	auto cbegin() const noexcept {
		return m_data.cbegin();
	}

	auto cend() const noexcept {
		return m_data.cend();
	}

	decltype(auto) operator[](int i) const {
		return m_data[i];
	}

	decltype(auto) operator[](int i) {
		return m_data[i];
	}

	void cancel() {
		m_is_canceled = true;
		m_cv.notify_all();
	}

private:
	void maybe_stop() const {
		if (m_is_canceled) {
			throw 1;//something random
		}
	}

	T do_pop() {
		auto retVal = std::move(m_data[0]);
		if constexpr (has_pop_front<container>::value)
			m_data.pop_front();
		else
			m_data.erase(m_data.begin());
		return retVal;
	}

	container m_data;
	std::mutex m_mut;
	std::condition_variable m_cv;
	std::atomic<bool> m_is_canceled = false;
};
