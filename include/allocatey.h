#pragma once
#include <utility>
#include "ref_counted.h"
#include "randomThings.h"

struct mem_pool_ :ref_counted {
	mem_pool_(size_t size) {
		start = static_cast<char*>(malloc(size));
		end = start;
		cap = start + size;
	}
	~mem_pool_() {
		if (start)
			free(start);
	}
	mem_pool_(mem_pool_&& other) noexcept {
		start = std::exchange(other.start, nullptr);
		end = std::exchange(other.end, nullptr);
		cap = std::exchange(other.cap, nullptr);
	}
	mem_pool_& operator=(mem_pool_&& other)noexcept {
		start = std::exchange(other.start, nullptr);
		end = std::exchange(other.end, nullptr);
		cap = std::exchange(other.cap, nullptr);
		return *this;
	}
	mem_pool_(const mem_pool_& other) = delete;
	mem_pool_& operator=(const mem_pool_&) = delete;
	template<typename T>
	T* allocate(const size_t n) {
		end = (char*)round_to_multiple((size_t)end, alignof(T));
		//if (end + sizeof(T)*n > cap)throw std::bad_alloc();
		return (T*)std::exchange(end, end + sizeof(T) * n);
	}

	char* start = nullptr;
	char* end = nullptr;
	char* cap = nullptr;
};

template<typename T = void>
struct single_chunk_allocator {
	using value_type = T;
	using difference_type = size_t;

	template<typename o> struct rebind { using other = single_chunk_allocator<o>; };

	single_chunk_allocator(size_t s = 2048):m_mem_pool(make_ref_count_ptr<mem_pool_>(s)){}

	single_chunk_allocator(ref_count_ptr<mem_pool_> t):m_mem_pool(std::move(t)) {}

	template<typename U>
	single_chunk_allocator(const single_chunk_allocator<U>& other) :m_mem_pool(other.m_mem_pool){
	}

	template<typename U>
	single_chunk_allocator(single_chunk_allocator<U>&& other) : m_mem_pool(other.m_mem_pool) {//move is the same as copy
	}

	template<typename U>
	single_chunk_allocator& operator=(const single_chunk_allocator<U>& other) {
		m_mem_pool = other.m_mem_pool;
		return *this;
	}

	single_chunk_allocator(single_chunk_allocator&& other)noexcept :m_mem_pool(other.m_mem_pool){}//same as copy

	single_chunk_allocator& operator=(single_chunk_allocator&& other) noexcept {//same as copy
		m_mem_pool = other.m_mem_pool;
		return *this;
	}
	single_chunk_allocator& operator=(const single_chunk_allocator&) = default;
	single_chunk_allocator(const single_chunk_allocator&) = default;

	~single_chunk_allocator() = default;

	T* allocate(size_t n) {
		return m_mem_pool->allocate<T>(n);
	}
	static void deallocate(T*, size_t) {}


private:
	ref_count_ptr<mem_pool_> m_mem_pool;
	template<typename U>
	friend struct single_chunk_allocator;
	friend struct single_chunk_mem_pool;
};

struct single_chunk_mem_pool{
	single_chunk_mem_pool(size_t s = 2048):m_stuff(make_ref_count_ptr<mem_pool_>(s)){}
	template<typename T>
	operator single_chunk_allocator<T>(){
		return single_chunk_allocator<T>(m_stuff);
	}
	single_chunk_allocator<void> to_allocator() {
		return single_chunk_allocator<void>(*this);//conversion
	}
private:
	ref_count_ptr<mem_pool_> m_stuff;
};
