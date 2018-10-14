#pragma once
#include "ref_counted.h"
#include <vector>
#include <mutex>
#include <algorithm>
#include "randomThings.h"

template<typename forward_it,typename binary_pred>
constexpr int num_consecutive(forward_it b, const forward_it e,binary_pred&& pred) {
	const auto it = std::adjacent_find(b, e,std::not_fn(pred));//it should point to a ;-;
	return std::distance(b, it) + (it != e);
}

template<typename forward_it,typename binary_pred>
constexpr forward_it find_consecutive(forward_it a,const forward_it end,int n,binary_pred&& pred) {
	for (; a + n < end;) {
		const auto c = num_consecutive(a, a + n, pred);
		if (c == n) 
			return a;
		a += c;
	}return end;
}

template<int block_size, int n>
struct pre_allocated_mem_pool:ref_counted{
	pre_allocated_mem_pool() {
		std::iota(m_free.begin(), m_free.end(), 0);
	}

	template<typename T>
	T* allocate(int count = 1) {
		if constexpr(sizeof(T) <= block_size) {
			if(count == 1){
				std::unique_lock<std::mutex> lock(m_mut,std::try_to_lock);
				if(lock && m_free.size()) {
					int q = m_free.back();
					m_free.pop_back();
					void* temp = &m_blocks[q];
					size_t space = block_size;
					temp = std::align(alignof(T), sizeof(T), temp, space);
					if(temp)
						return (T*)temp;
					//else
					m_free.push_back(q);
					return (T*)malloc(sizeof(T));
				}
			}else {
				std::unique_lock<std::mutex> lock(m_mut, std::try_to_lock);
				if (lock && m_free.size()*block_size >= count * sizeof(T)) {
					const int num_required_consecutive = ceilDiv(count * sizeof(T), block_size);					
					std::sort(m_free.begin(),m_free.end());

					const auto start_of_block = find_consecutive(m_free.begin(), m_free.end(), num_required_consecutive, [](auto a, auto b) {return b == a + 1; });
					if(start_of_block != m_free.end()) {
						size_t space = block_size * num_required_consecutive;
						void* ptr = &m_blocks[*start_of_block];
						ptr = std::align(alignof(T), sizeof(T) * count, ptr, space);
						if (ptr) {
							auto it2 = std::rotate(m_free.begin() + *start_of_block, m_free.begin() + *start_of_block + num_required_consecutive, m_free.end());
							m_free.erase(it2, m_free.end());
							return (T*)ptr;
						}
					}
				}
			}
		}
		return (T*)malloc(sizeof(T) * count);	
	}

	template<typename T>
	void deallocate(T* ptr,int count = 1) {
		if constexpr(sizeof(T) <= block_size) {
			if(ptr >= (T*)&m_blocks[0] && ptr <=(T*)&m_blocks.back()) {
				//unalign the pointer then get it's distance from 0
				const int index = (std::aligned_storage_t<block_size>*)(((int)ptr / 32) * 32) - &m_blocks[0];
				std::unique_lock<std::mutex> lock(m_mut);
				const int num_required_consecutive = ceilDiv(count * sizeof(T), block_size);
				for (int i = index; i < index + num_required_consecutive; ++i)
					m_free.push_back(i);
			}else {
				free(ptr);
			}
		}else{
			free(ptr);
		}
	}

private:
	std::mutex m_mut;
	
	std::vector<int> m_free = std::vector<int>(n);
	std::vector<std::aligned_storage_t<block_size>> m_blocks = std::vector<std::aligned_storage_t<block_size>>(n);
};



template<typename T,int block_size,int num>
struct preallocated_allocator_n{
	//T* allocate(int n) {}


};

template<typename T>
using preallocated_allocator = preallocated_allocator_n<T, 32, 1024>;