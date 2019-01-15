#pragma once
#include "bytell_hash_map.hpp"
#include "indirect.h"

template<typename K,typename V,typename H = std::hash<K>,typename E = std::equal_to<>,typename A = std::allocator<std::pair<const K,indirect<V>>>>
struct rename_later_4{
	using map_t = ska::bytell_hash_map<K, indirect<V>, H, E, A>;
	using value_type = std::pair<const K , V>;
	using reference = std::pair<const K&, V&>;
	using const_reference = std::pair<const K&, const V&>;
	
	template<typename value_t,typename iterator_t>
	struct templated_iterator{
		templated_iterator(iterator_t it,iterator_t end):m_it(std::move(it)) ,m_end(std::move(end)) {
			set_stuff();
		}

		template<typename value_t2, typename iterator_t2, std::enable_if_t<std::is_convertible_v<iterator_t2,iterator_t>> = 0>
		explicit templated_iterator(templated_iterator<value_t2,iterator_t2> other):templated_iterator(other.m_it,other.m_first) {
			
		}

		value_t& operator*() const{
			return const_cast<value_t&>(*m_stuff);
		}

		value_t* operator->() const{
			return &(**this);
		}

		templated_iterator& operator++() {
			++m_it;
			set_stuff();
			return *this;
		}

		templated_iterator operator++(int) {
			auto t = *this;			
			++m_it;
			return t;
		}

		templated_iterator& operator--() {
			--m_it;
			set_stuff();
			return *this;
		}

		templated_iterator operator--(int) {
			auto t = *this;	
			--m_it;
			return t;
		}

		template<typename U,typename other_it>
		bool operator==(const templated_iterator<U,other_it>& other) const{
			return m_it == other.m_it;
		}

		template<typename U, typename other_it>
		bool operator!=(const templated_iterator<U, other_it>& other) const{
			return m_it != other.m_it;
		}

	private:
		void set_stuff() {
			m_stuff = std::nullopt;
			if (m_it != m_end) {
				auto& a = m_it->first;
				auto& b = m_it->second;
				m_stuff.emplace(a,b);
			}
		}
		iterator_t m_it;
		iterator_t m_end;
		std::optional<value_t> m_stuff;//;-;

		friend struct rename_later_4;
		template<typename, typename> friend struct templated_iterator;
	};

	struct node_handle {
		K& key() noexcept {
			return m_data.first;
		}
		const K& key()const noexcept {
			return m_data.first;
		}

		V& mapped() noexcept {
			return *m_data.second;
		}
		const V& mapped()const noexcept {
			return *m_data.second;
		}
	private:
		friend struct rename_later_4;
		std::pair<K, indirect<V>> m_data;
	};

	using iterator = templated_iterator<reference, typename map_t::iterator>;
	using const_iterator = templated_iterator<const_reference, typename map_t::const_iterator>;
	
	std::pair<iterator,bool> insert(std::pair<K,V> thing) {
		indirect<V> key = std::move(thing.second);
		const auto[it, succcess] = m_data.insert(std::make_pair(std::move(thing.first), std::move(key)));
		return {iterator(it,m_data.end()),succcess};
	}

	std::pair<iterator, bool> insert(std::pair<K, indirect<V>> thing) {		
		const auto[it, succcess] = m_data.insert(std::move(thing));
		return { iterator(it,m_data.end()),succcess };
	}

	iterator insert(node_handle& h) {
		return insert(std::move(h.m_data)).first;
	}

	V& operator[](const K& key) {
		return *m_data[key];
	}

	V& at(const K& key) {
		return *m_data.at(key);
	}

	const V& at(const K& key) const{
		return *m_data.at(key);
	}

	iterator find(const K& key) noexcept{
		return iterator(m_data.find(key),m_data.end());
	}

	const_iterator find(const K& key) const noexcept {
		return const_iterator(m_data.find(key),m_data.end);
	}

	bool contains(const K& key)const noexcept {
		return find(key) != end();
	}

	void erase(const K& key) {
		m_data.erase(key);
	}

	iterator erase(iterator a,iterator b) {
		return { m_data.erase(a.m_it,b.m_it),m_data.end() };
	}

	iterator erase(const_iterator a, const_iterator b) {
		return { m_data.erase(a.m_it,b.m_it),m_data.end() };
	}

	void clear() noexcept{
		m_data.clear();
	}

	size_t size()const noexcept {
		return m_data.size();
	}

	bool empty()const noexcept {
		return size() == 0;
	}

	void reserve(size_t n) {
		m_data.reserve(n);
	}

	bool operator==(const rename_later_4& other) {
		return m_data == other.m_data;
	}
	bool operator!=(const rename_later_4& other) {
		return m_data != other.m_data;
	}

	template<typename K_,typename... args>
	std::pair<iterator,bool> emplace(K_&& key,args&&... Args) {
		return m_data.emplace(std::forward<K_>(key), std::forward<args>(Args)...);
	}

	iterator begin() {
		return iterator(m_data.begin(),m_data.end());
	}

	iterator end() {
		return iterator(m_data.end(),m_data.end());
	}

	const_iterator begin() const{
		return const_iterator(m_data.begin(),m_data.end());
	}

	const_iterator end() const{
		return const_iterator(m_data.end(), m_data.end());
	}

	void erase(const iterator& it) {
		m_data.erase(it.m_it);
	}

	void erase(const const_iterator& it) {
		m_data.erase(it.m_it);
	}

	node_handle extract(const K& k) {
		auto it = m_data.find(k);
		node_handle retVal;
		if (it != m_data.end()) {
			retVal.m_data.first = std::move(const_cast<K&>((it->first)));
			retVal.m_data.second= std::move((it->second));
			m_data.erase(it);
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
	std::pair<iterator,bool> insert_or_assign(const K& key,Value&& v) {
		const auto[it, s] = m_data.insert_or_assign(key,std::forward<Value>(v));
		return { iterator(it,m_data.end()),s };
	}

	template<typename Value>
	std::pair<iterator, bool> insert_or_assign(K&& key, Value&& v) {
		const auto[it, s] = m_data.insert_or_assign(key, std::forward<Value>(v));
		return { iterator(it,m_data.end()),s };
	}

private:
	map_t m_data;
};

inline void asdhasdjkasdh() {
	rename_later_4<int, int> blargus;
	indirect<int> b = 2;
	int q = b;
	blargus[4] = 2;
	blargus.insert(std::make_pair(1, 23));
	auto t = blargus.begin();

	for(const auto& i:blargus) {
		i.second = 1;//;-;
	}
}


