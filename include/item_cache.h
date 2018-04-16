#pragma once
#include <vector>
#include <deque>
#include <array>
#include <memory>

template<typename T,size_t size>
class static_item_cache{
public:
	void add(T item) {
		m_cache[m_position++] = std::move(item);
		m_position = m_position % size;
	}
	std::array<T, size>& cache() noexcept { return m_cache; }
	const std::array<T, size>& cache() const noexcept{ return m_cache; }
private:
	std::array<T, size> m_cache;
	int m_position = 0;
};

template<typename T>
class dynamic_item_cache{
public:
	dynamic_item_cache(const size_t cache_size, const bool reserve_before = false):m_cache_size(cache_size) {
		if (reserve_before)
			m_cache.reserve(m_cache_size);
	}
	void add(T item) {
		if (m_cache.size()<m_cache_size) {
			m_cache.push_back(std::move(item));
		}else {
			m_cache[m_position++] = std::move(item);
			m_position %= m_cache_size;
		}
	}
	void cache_size(size_t t_cache_size) noexcept{
		m_cache_size = t_cache_size;
	}
	size_t cache_size() const noexcept{
		return m_cache_size;
	}
	std::vector<T>& data()noexcept { return m_cache; }
	const std::vector<T>& data()const noexcept { return m_cache; }
private:
	std::vector<T> m_cache;
	size_t m_cache_size = 10;
	int64_t m_position = 0;
};

template<typename T, size_t size>
class static_heap_allocated_item_cache {
public:
	void add(T item) {
		m_cache[m_position++] = std::make_unique<T>(std::move(item));
		m_position = m_position % size;
	}
	std::array<std::unique_ptr<T>, size>& cache() noexcept { return m_cache; }
	const std::array<std::unique_ptr<T>, size>& cache() const noexcept { return m_cache; }
private:
	std::array<std::unique_ptr<T>, size> m_cache;
	int m_position = 0;
};