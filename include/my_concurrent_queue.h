#pragma once
#include <deque>
#include <condition_variable>

template<typename T, template<typename...> typename container = std::deque>
class my_concurrent_queue{
public:
	template<typename U,typename = void>
	struct has_pop_front:std::false_type{};

	template<typename U>
	struct has_pop_front<U,std::void_t<decltype(std::declval<U>().pop_front())>> :std::true_type {};

	void push(T t) {
		std::lock_guard<std::mutex> locky(m_mut);
		m_data.push_back(std::move(t));
		m_cv.notify_one();
	};
	template<typename... args>
	void emplace(args&&...Args) {
		std::lock_guard<std::mutex> locky(m_mut);
		m_data.emplace_back(std::forward<args>(Args)...);
		m_cv.notify_one();
	};
	T pop() {
		std::unique_lock<std::mutex> locky(m_mut);
		m_cv.wait(locky, [&]() {return m_data.size(); });
		T retVal = std::move(m_data[0]);
		if constexpr(has_pop_front<container<T>>::value)
			m_data.pop_front();
		else
			m_data.erase(m_data.begin());
		return retVal;
	};
	size_t size()const noexcept {
		return m_data.size();
	}
private:
	container<T> m_data;
	std::mutex m_mut;
	std::condition_variable m_cv;
};

