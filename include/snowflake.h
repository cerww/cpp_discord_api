#pragma once
#include <nlohmann/json.hpp>
#include <utility>
#include <type_traits>
#include "allocatey.h"

struct snowflake{
	size_t val = 0;
	bool operator==(const snowflake other) const noexcept{ return val == other.val; }
	snowflake() = default;
	~snowflake() = default;
	snowflake& operator=(snowflake&& other)noexcept { val = std::exchange(other.val, 0); return *this; }
	snowflake& operator=(const snowflake& other) = default;
	snowflake(snowflake&& other)noexcept :val(other.val) { other.val = 0; }
	snowflake(const snowflake& other) = default;
};

inline bool operator!=(const snowflake a,const snowflake b) {
	return a.val != b.val;
}

inline bool operator<(const snowflake a,const snowflake b) {
	return a.val < b.val;
}

inline bool operator>(const snowflake a, const snowflake b) {
	return a.val > b.val;
}

inline bool operator<=(const snowflake a, const snowflake b) {
	return a.val <= b.val;
}

inline bool operator>=(const snowflake a, const snowflake b) {
	return a.val >= b.val;
}

inline void to_json(nlohmann::json& json,const snowflake& wawt) {
	json = std::to_string(wawt.val);
}

inline void from_json(const nlohmann::json& in,snowflake& out) {
	if (in.is_null()) return;
	try{
		out.val = std::stoull(in.get_ref<const std::string&>());
	}catch(...) {}
}

namespace std{
	template<>
	struct hash<snowflake>{	
		size_t operator()(snowflake s)const{
			return std::hash<size_t>()(s.val);
		}
	};
}

template<typename T,typename = void>
struct has_id:std::false_type{};

template<typename T>
struct has_id<T, std::void_t<decltype(std::declval<T>().id())>>{
	static constexpr bool value_type = std::is_same_v<decltype(std::declval<T>().id()), snowflake>;
};

template<typename T>
static constexpr bool has_id_v = has_id<T>::value_type;

template<typename T>
std::enable_if_t<has_id_v<T>,std::unordered_map<snowflake,T*,std::hash<snowflake>,std::equal_to<>,single_chunk_allocator<std::pair<const snowflake,T*>>>> to_map(std::vector<T>& stuffs) {
	single_chunk_mem_pool pool(std::max(stuffs.size() * 64,2048ull));
	std::unordered_map<snowflake, T*, std::hash<snowflake>, std::equal_to<>, single_chunk_allocator<std::pair<const snowflake, T*>>> retVal(pool);//so long ;-;
	retVal.reserve(stuffs.size());
	for(auto& item:stuffs) 
		retVal[item.id()] = &item;
	return retVal;
}

struct id_equal_to{
	id_equal_to(snowflake i) : id(i) {};
	template<typename T>
	bool operator()(const T& thing){
		return thing.id() == id;
	}
	const snowflake id;
};
//just use auto
template<typename T>
struct rename_later_3{//i need a better name
	rename_later_3(std::unordered_map<snowflake,T>& a,const std::vector<snowflake>& b):m_wat(a),m_waty(b){};

	struct iterator{
		iterator(rename_later_3* t_parent,std::vector<snowflake>::const_iterator t_it):
		m_parent(t_parent),
		it(t_it){}
		iterator& operator++() {
			++it;
			return *this;
		}
		iterator& operator++(int) {
			++it;
			return *this;
		}
		T& operator*() {
			return m_parent->m_wat[*it];
		}
		const T& operator*() const {
			return m_parent->m_wat.at(*it);
		}
		iterator& operator--() {
			++it;
			return *this;
		}
		iterator& operator--(int) {
			++it;
			return *this;
		}
		bool operator==(iterator other) {
			return it == other.it;
		}
		bool operator!=(iterator other) {
			return it != other.it;
		}
		size_t operator-(iterator other) {
			return it - other.it;
		}
		iterator& operator+=(size_t i) {
			it += i;
			return *this;
		}
		iterator& operator-=(size_t i) {
			it += i;
			return *this;
		}
		iterator operator+(size_t i) {
			iterator retVal = *this;			
			return retVal+=i;
		}
		iterator operator-(size_t i) {
			iterator retVal = *this;
			return retVal -= i;
		}
		decltype(auto) operator[](size_t i)const {
			return it[i];
		}decltype(auto) operator[](size_t i) {
			return it[i];
		}
	private:
		rename_later_3 * m_parent;
		std::vector<snowflake>::const_iterator it;
	};
	struct const_iterator {
		const_iterator(rename_later_3* t_parent, std::vector<snowflake>::const_iterator t_it) :
			m_parent(t_parent),
			it(t_it) {}
		const_iterator& operator++() {
			++it;
			return *this;
		}
		const_iterator& operator++(int) {
			++it;
			return *this;
		}		
		const T& operator*() const {
			return m_parent->m_wat.at(*it);
		}
		const_iterator& operator--() {
			++it;
			return *this;
		}
		const_iterator& operator--(int) {
			++it;
			return *this;
		}
		bool operator==(const_iterator other) {
			return it == other.it;
		}
		bool operator!=(const_iterator other) {
			return it != other.it;
		}
		size_t operator-(const_iterator other) {
			return it - other.it;
		}
		const_iterator& operator+=(size_t i) {
			it += i;
			return *this;
		}
		const_iterator& operator-=(size_t i) {
			it += i;
			return *this;
		}
		const_iterator operator+(const size_t i) {
			const_iterator retVal = *this;
			return retVal += i;
		}
		const_iterator operator-(const size_t i) {
			const_iterator retVal = *this;
			return retVal -= i;
		}
		decltype(auto) operator[](size_t i)const{
			return it[i];
		}
		decltype(auto) operator[](size_t i) {
			return it[i];
		}
	private:
		rename_later_3 * m_parent;
		std::vector<snowflake>::const_iterator it;
	};
	
	
	iterator begin() {
		return iterator{ this,m_waty.begin() };
	}
	iterator end() {
		return iterator{ this,m_waty.end() };
	}
	const_iterator begin() const {
		return const_iterator{ this,m_waty.begin() };
	}
	const_iterator end() const{
		return const_iterator{ this,m_waty.end() };
	}
	T& operator[](size_t i){
		return m_wat[m_waty[i]];
	}
	const T& operator[](size_t i)const {
		return m_wat.at(m_waty[i]);
	}
	size_t size()const noexcept { return m_waty.size(); }
	operator std::vector<T>(){
		std::vector<T> retVal;
		retVal.reserve(size());
		for(auto i:m_waty) 
			retVal.push_back(m_wat[i]);
		return retVal;
	}
private:
	std::unordered_map<snowflake, T>& m_wat;
	const std::vector<snowflake>& m_waty;
	friend struct iterator;
	friend struct const_iterator;
};

template<typename T>
struct from_id_to{
	T& operator()(snowflake s){
		return thing[s];
	}
	std::unordered_map<snowflake, T>& thing;
};

const auto transform_to_pair = [](auto&& a, auto&& fn1,auto&& fn2){
	auto ret = fn1(a);
	auto other = fn2(ret);
	return std::make_pair(std::move(other), std::move(ret));
};

template<typename T>
std::pair<snowflake,T> get_with_id(const nlohmann::json& json) {
	auto ret = json.get<T>();
	const snowflake id = ret.id();
	return { id,std::move(ret) };
}

static inline const auto id_comp = [](auto&& a, snowflake b) {return a.id() < b; };

static inline const auto get_id = [](auto&& a) {return a.id(); };
