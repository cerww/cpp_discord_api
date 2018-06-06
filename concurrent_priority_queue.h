#pragma once
#include <vector>
#include <mutex>
#include <algorithm>

template<typename T,typename comp,typename container = std::vector<T>>
struct concurrent_priority_queue{

	void insert(T thing) {
		std::lock_guard<std::mutex> lock(m_mut);
		m_data.insert(std::upper_bound(m_data.begin(), m_data.end(), thing, comp{}),std::move(thing));
		m_cv.notify_one();
	}

	T& operator[](int i){
		return m_data[i];
	}

	const T& operator[](int i) const{
		return m_data[i];
	}
private:
	std::condition_variable m_cv;
	std::mutex m_mut;
	container m_data;
};
