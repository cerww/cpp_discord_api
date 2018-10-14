#pragma once
#include <nlohmann/json.hpp>
#include <utility>
#include <type_traits>
#include "allocatey.h"
#include "bytell_hash_map.hpp"
#include "indirect.h"
#include "rename_later_4.h"
#include <charconv>

struct snowflake{
	size_t val = 0;
	bool operator==(const snowflake& other) const noexcept{ return val == other.val; }
	snowflake() = default;
	~snowflake() = default;
	snowflake& operator=(snowflake&& other)noexcept { val = std::exchange(other.val, 0); return *this; }
	snowflake& operator=(const snowflake& other) = default;
	snowflake(snowflake&& other)noexcept :val(other.val) { other.val = 0; }
	snowflake(const snowflake& other) = default;
};

inline bool operator!=(const snowflake& a,const snowflake& b) {
	return a.val != b.val;
}

inline bool operator<(const snowflake& a,const snowflake& b) {
	return a.val < b.val;
}

inline bool operator>(const snowflake& a, const snowflake& b) {
	return a.val > b.val;
}

inline bool operator<=(const snowflake& a, const snowflake& b) {
	return a.val <= b.val;
}

inline bool operator>=(const snowflake& a, const snowflake& b) {
	return a.val >= b.val;
}

inline void to_json(nlohmann::json& json,const snowflake& wawt) {
	char buffer[20] = {};
	//json = std::to_string(wawt.val);
	const auto r = std::to_chars(buffer, buffer + 20, wawt.val);
	json = std::string(buffer, r.ptr);
}

inline void from_json(const nlohmann::json& in,snowflake& out) {
	if (in.is_null()) return;
	try{
		//out.val = std::stoull(in.get_ref<const std::string&>());
		const auto& str = in.get_ref<const std::string&>();
		std::from_chars(str.data(), str.data() + str.size(), out.val);		
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
struct has_id<T, std::void_t<decltype(std::declval<T>().id())>> :std::is_same<decltype(std::declval<T>().id()), snowflake> {};

template<typename T>
static constexpr bool has_id_v = has_id<T>::value;

template<typename T>//requires has_id_v<T>
std::enable_if_t<has_id_v<T>,ska::bytell_hash_map<snowflake,T*,std::hash<snowflake>,std::equal_to<>,single_chunk_allocator<std::pair<const snowflake,T*>>>> to_map(std::vector<T>& stuffs) {
	single_chunk_mem_pool pool(std::max(stuffs.size() * sizeof(std::pair<const snowflake, T*>) * 2,2048ull));//2048 and 2 are random numbers
	ska::bytell_hash_map<snowflake, T*, std::hash<snowflake>, std::equal_to<>, single_chunk_allocator<std::pair<const snowflake, T*>>> retVal(std::move(pool));//so long ;-;
	retVal.reserve(stuffs.size());
	for(auto& item:stuffs) 
		retVal[item.id()] = &item;
	return retVal;
}

struct id_equal_to{
	explicit id_equal_to(snowflake i) : id(std::move(i)) {};
	template<typename T>
	bool operator()(const T& thing){
		return thing.id() == id;
	}
	const snowflake id;
};

//this is like a "map" view ;-;, i should just make one, then typedef this
template<typename T>
struct discord_obj_map{
	discord_obj_map() = default;
	discord_obj_map(const rename_later_4<snowflake,T>& data):m_data(&data){}
	
	template<typename it>
	struct templated_iterator{
		templated_iterator(it o):m_it(std::move(o)){}

		const T& operator*() const{
			return *m_it;
		}

		const T* operator->()const {
			return &*m_it;
		}

		templated_iterator& operator++()noexcept {
			++m_it;
			return *this;
		}

		templated_iterator operator++(int) noexcept {
			auto other = *this;
			++m_it;
			return other;
		}

		template<typename o>
		bool operator==(const templated_iterator<o>& other)const noexcept {
			return m_it == other.m_it;
		}
		
		template<typename o>
		bool operator!=(const templated_iterator<o>& other)const noexcept {
			return m_it != other.m_it;
		}

	private:
		it m_it;
		template<typename>
		friend struct templated_iterator;
	};

	using iterator = templated_iterator<typename rename_later_4<snowflake, T>::const_iterator>;
	using const_iterator = iterator;

	const T& operator[](snowflake s)const{
		return m_data->at(s);
	}

	const T& at(snowflake s) const{
		return m_data->at(s);
	}

	iterator begin()const noexcept {
		return m_data->begin();
	}

	iterator end()const noexcept {
		return m_data->end();
	}

	const_iterator cbegin()const noexcept {
		return m_data->begin();
	}

	const_iterator cend()const noexcept {
		return m_data->end();
	}

	iterator find(snowflake s) const noexcept{
		return m_data->find(s);
	}

	size_t size()const noexcept {
		return m_data->size();
	}
private:
	const rename_later_4<snowflake, T>* m_data;
};


//just use auto
template<typename T>
struct discord_obj_list{
	discord_obj_list() = default;
	explicit discord_obj_list(const discord_obj_map<T>& a,const std::vector<snowflake>& b):m_wat(a),m_waty(&b){};
	
	template<typename value_type,int stride>
	struct iterator_{
		explicit iterator_(discord_obj_list* t_parent, const std::vector<snowflake>::const_iterator t_it):
		m_parent(t_parent),
		m_it(t_it){}

		iterator_& operator++() {
			m_it += stride;
			return *this;
		}

		iterator_ operator++(int) {
			auto other = *this;			
			m_it += stride;
			return other;
		}

		std::add_const_t<value_type>& operator*() const noexcept{
			return m_parent->m_wat.at(*m_it);
		}

		const value_type* operator->()const noexcept {
			return &m_parent->m_wat.at(*m_it);
		}

		iterator_& operator--() noexcept{
			m_it -= stride;
			return *this;
		}
		iterator_ operator--(int) noexcept {
			auto other = *this;			
			m_it -= stride;
			return other;
		}
		template<typename O,int N>
		bool operator==(const iterator_<O, N>& other) {
			return m_it == other.m_it;
		}
		template<typename O,int N>
		bool operator!=(const iterator_<O, N>& other) {
			return m_it != other.m_it;
		}

		template<typename O,int N>
		size_t operator-(const iterator_<O, N>& other) {
			return m_it - other.m_it;
		}

		iterator_& operator+=(size_t i) {
			m_it += i * stride;
			return *this;
		}
		iterator_& operator-=(size_t i) {
			m_it -= i * stride;
			return *this;
		}
		iterator_ operator+(size_t i) {
			iterator_ retVal = *this;
			return retVal+=i;
		}
		iterator_ operator-(size_t i) {
			iterator_ retVal = *this;
			return retVal -= i;
		}
		decltype(auto) operator[](size_t i)const {
			return m_parent->m_wat.at(m_it[i]);
		}
		decltype(auto) operator[](size_t i) {
			return m_parent->m_wat.at(m_it[i]);
		}
	private:
		discord_obj_list * const m_parent;
		std::vector<snowflake>::const_iterator m_it;
	};

	using iterator = iterator_<T,1>;
	using const_iterator = iterator_<const T,1>;

	using reverse_iterator = iterator_<T, -1>;
	using const_reverse_iterator = iterator_<const T, -1>;

	iterator begin() {
		return iterator{ this,ids().begin() };
	}
	iterator end() {
		return iterator{ this,ids().end() };
	}
	const_iterator begin() const {
		return const_iterator{ this,ids().begin() };
	}
	const_iterator end() const{
		return const_iterator{ this,ids().end() };
	}
	T& operator[](size_t i){
		return m_wat[ids()[i]];
	}
	const T& operator[](size_t i)const {
		return m_wat.at(ids()[i]);
	}
	size_t size()const noexcept { return ids().size(); }

	operator std::vector<T>()const{
		std::vector<T> retVal;
		retVal.reserve(size());
		for(const auto& i: ids())
			retVal.push_back(m_wat.at(i));
		return retVal;
	}

private:
	const std::vector<snowflake>& ids()const noexcept {
		return *m_waty;
	}
	discord_obj_map<T> m_wat;
	std::vector<snowflake> const* m_waty = nullptr;
};
//template<typename T,template<typename,typename...> typename map_t> discord_obj_list(map_t<snowflake, T>& a, const std::vector<snowflake>& b)->discord_obj_list<T, map_t>;

const auto transform_to_pair = [](auto&& a, auto&& fn1,auto&& fn2){
	//evaluation order is undefined ;-;, so i can't inline this
	auto ret = std::invoke(std::forward(fn1),a);
	auto other = std::invoke(std::forward(fn2),ret);
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

