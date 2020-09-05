#pragma once
#include <memory>
#include <vector>
#include "../common/ref_count_ptr.h"
#include "../common/crtp_stuff.h"
#include <stdexcept>
#include "../common/bytell_hash_map.hpp"
#include <range/v3/utility/common_tuple.hpp>
#include <range/v3/all.hpp>
#include <range/v3/core.hpp>
#include "../common/arrow_proxy.h"


template<typename T>
struct thread_unsafe_ref_counted_but_different:crtp<T> {
private:	
	void increment_ref_count() const noexcept{
		++m_ref_count;
	}

	bool decrement_ref_count() const noexcept{
		return --m_ref_count <= this->self().get_cyclic_ref_count();
	}
	
	mutable int m_ref_count = 0;
	template<typename>
	friend struct ref_count_ptr;
};

template<typename F,typename S>
std::pair<F,ref_count_ptr<S>> to_thingy_that_works(std::pair<F,S> thing) {
	return std::make_pair(std::move(thing.first), make_ref_count_ptr<S>(std::move(thing.second)));
}

template<typename K, typename V, typename H = std::hash<K>, typename E = std::equal_to<>, typename A = std::allocator<std::pair<const K, ref_count_ptr<V>>>>
struct ref_stable_map2 {
	using map_t = ska::bytell_hash_map<K, ref_count_ptr<V>, H, E, A>;
	//using map_t = ska::bytell_hash_map<K, std::unique_ptr<V>, H, E, A>;
	using value_type = std::pair<const K, V>;
	using reference = ranges::common_pair<const K&, V&>;//can't use std::pair for some reason
	using const_reference = ranges::common_pair<const K&, const V&>;
	using allocator_type = A;

	ref_stable_map2() = default;

	//TODO conceptfy this when i can
	template<typename it, typename sentinal_t>
	explicit ref_stable_map2(it i, sentinal_t sentinal) {

		if constexpr (ranges::sized_sentinel_for<it, sentinal_t>) {
			m_data.reserve(ranges::iter_size(i, sentinal));
		}
		insert(i, sentinal);
	}

	template<typename reference_type_t, typename iterator_t>
	struct templated_iterator {
		using iterator_category = std::forward_iterator_tag;
		using difference_type = ptrdiff_t;
		using value_type = std::pair<const K, V>;
		using reference = reference_type_t;
		using pointer = arrow_proxy<reference>;

		templated_iterator() = default;

		templated_iterator(iterator_t it) :
			m_it(std::move(it)) {}

		template<typename value_t2, typename iterator_t2, std::enable_if_t<std::is_convertible_v<iterator_t2, iterator_t>> = 0>
		explicit templated_iterator(templated_iterator<value_t2, iterator_t2> other) :
			templated_iterator(other.m_it) {}

		reference operator*() const {
			return reference((*m_it).first, *(*m_it).second);
		}

		pointer operator->() const {
			return pointer({ **this });
		}

		templated_iterator& operator++() {
			++m_it;
			return *this;
		}

		templated_iterator operator++(int) {
			auto t = *this;
			++m_it;
			return t;
		}

		template<typename U, typename other_it>
		bool operator==(const templated_iterator<U, other_it>& other) const noexcept {
			return m_it == other.m_it;
		}

		template<typename U, typename other_it>
		bool operator!=(const templated_iterator<U, other_it>& other) const noexcept {
			return m_it != other.m_it;
		}

	private:
		iterator_t m_it;

		friend struct ref_stable_map2;
		template<typename, typename>
		friend struct templated_iterator;
	};

	struct node_handle {
		K& key() noexcept {
			return m_data.first;
		}

		const K& key() const noexcept {
			return m_data.first;
		}

		V& mapped() noexcept {
			return *m_data.second;
		}

		const V& mapped() const noexcept {
			return *m_data.second;
		}

		ref_count_ptr<V>& mapped_ptr() noexcept {
			return m_data.second;
		}

		const ref_count_ptr<V>& mapped_ptr() const noexcept {
			return m_data.second;
		}

	private:
		friend struct ref_stable_map2;
		std::pair<K, ref_count_ptr<V>> m_data;
	};

	using iterator = templated_iterator<reference, typename map_t::iterator>;
	using const_iterator = templated_iterator<const_reference, typename map_t::const_iterator>;

	iterator begin() {
		return iterator(m_data.begin());
	}

	iterator end() {
		return iterator(m_data.end());
	}

	const_iterator begin() const {
		return const_iterator(m_data.begin());
	}

	const_iterator end() const {
		return const_iterator(m_data.end());
	}

	const_iterator cbegin() const {
		return const_iterator(m_data.begin());
	}

	const_iterator cend() const {
		return const_iterator(m_data.end());
	}

	/*
	template<typename I, typename S>
	void insert(I&& it, S&& sent) {
		m_data.insert(std::forward<I>(it), std::forward<S>(sent));
	}

	void insert(std::initializer_list<value_type> il) {
		insert(il.begin(), il.end());
	}
	*/
	
	std::pair<iterator, bool> insert(value_type thing) {
		ref_count_ptr<V> key = make_ref_count_ptr<V>(std::move(thing.second));
		const auto [it, succcess] = m_data.insert(std::make_pair(std::move(thing.first), std::move(key)));
		return { iterator(it), succcess };
	}

	std::pair<iterator, bool> insert(std::pair<K,ref_count_ptr<V>> thing) {
		ref_count_ptr<V> key = make_ref_count_ptr<V>(std::move(thing.second));
		const auto [it, succcess] = m_data.insert(std::make_pair(std::move(thing.first), std::move(key)));
		return { iterator(it), succcess };
	}

	iterator insert(node_handle& h) {
		return insert(std::move(h.m_data)).first;
	}

	template<typename Key, typename... Args>
	std::pair<iterator, bool> emplace(Key&& key, Args&&... args) {
		return m_data.emplace(std::forward<Key>(key), make_ref_count_ptr<V>(std::forward<Args>(args)...));
	}

	iterator insert(const_iterator, value_type value) {
		return insert(to_thingy_that_works(std::move(value))).first;
	}

	V& operator[](const K& key) {		
		auto& ret = m_data[key];
		if (!ret) {
			ret = make_ref_count_ptr<V>();
		}
		return *ret;
	}

	V& at(const K& key) {
		return *m_data.at(key);
	}

	const V& at(const K& key) const {
		return *m_data.at(key);
	}

	iterator find(const K& key) noexcept {
		return iterator(m_data.find(key));
	}

	const_iterator find(const K& key) const noexcept {
		return const_iterator(m_data.find(key));
	}

	bool contains(const K& key) const noexcept {
		return find(key) != end();
	}

	void erase(const K& key) {
		m_data.erase(key);
	}

	template<typename T, typename I>
	void erase(templated_iterator<T, I> it) {
		m_data.erase(it.m_it);
	}

	template<typename T, typename I>
	void erase(templated_iterator<T, I> a, templated_iterator<T, I> b) {
		m_data.erase(a.m_it, b.m_it);
	}

	void clear() noexcept {
		m_data.clear();
	}

	size_t size() const noexcept {
		return m_data.size();
	}

	size_t max_size() const noexcept {
		return m_data.max_size();
	}

	float load_factor() const {
		return m_data.load_factor();
	}

	void max_load_factor(float value) {
		m_data.max_load_factor(value);
	}

	bool empty() const noexcept {
		return size() == 0;
	}

	void reserve(size_t n) {
		m_data.reserve(n);
	}

	bool operator==(const ref_stable_map2& other) {
		return m_data == other.m_data;
	}

	bool operator!=(const ref_stable_map2& other) {
		return m_data != other.m_data;
	}

	node_handle extract(const K& k) {
		auto it = m_data.find(k);
		node_handle retVal;
		if (it != m_data.end()) {
			retVal.m_data.first = std::move(const_cast<K&>((it->first)));
			retVal.m_data.second = std::move((it->second));
			m_data.erase(it);
		}
		else {
			throw std::out_of_range("");
		}
		return retVal;
	}

	node_handle extract(const iterator& it) {
		node_handle retVal;
		if (it.m_it != m_data.end()) {
			retVal.m_data.first = std::move(const_cast<K&>((it.m_it->first)));
			retVal.m_data.second = std::move((it.m_it->second));
			m_data.erase(it.m_it);
		}
		return retVal;
	}

	node_handle extract(const const_iterator& it) {
		node_handle retVal;
		if (it.m_it != m_data.end()) {
			retVal.m_data.first = std::move(const_cast<K&>((it.m_it->first)));
			retVal.m_data.second = std::move((it.m_it->second));
			m_data.erase(it.m_it);
		}
		return retVal;
	}

	template<typename Value>
	std::pair<iterator, bool> insert_or_assign(const K& key, Value&& v) {
		const auto [it, s] = m_data.insert_or_assign(key, make_ref_count_ptr<V>(std::forward<Value>(v)));
		return { iterator(it), s };
	}

	template<typename Value>
	std::pair<iterator, bool> insert_or_assign(K&& key, Value&& v) {
		const auto [it, s] = m_data.insert_or_assign(std::move(key), make_ref_count_ptr<V>((std::forward<Value>(v))));
		return { iterator(it), s };
	}

	void shrink_to_fit() {
		m_data.shrink_to_fit();
	}

	void rehash(size_t num_items) {
		m_data.rehash(num_items);
	}

	auto& get_allocator() {
		return m_data.get_allocator();
	}

	const auto& get_allocator() const {
		return m_data.get_allocator();
	}


private:
	map_t m_data;
};
