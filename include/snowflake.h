#pragma once
#include <nlohmann/json.hpp>
#include <utility>
#include <type_traits>
#include "allocatey.h"
#include "bytell_hash_map.hpp"
#include "indirect.h"
#include "rename_later_4.h"
#include <charconv>
#include "iterator_facade.h"

struct snowflake{
	size_t val = 0;
	constexpr bool operator==(const snowflake& other) const noexcept{ return val == other.val; }
	constexpr snowflake() = default;
	~snowflake() = default;
	constexpr snowflake& operator=(snowflake&& other)noexcept {
		//std::swap isn't constexpr
		auto t = other.val;
		other.val = val;
		val = t;		
		return *this;
	}
	constexpr snowflake& operator=(const snowflake& other) = default;
	constexpr snowflake(snowflake&& other)noexcept :val(other.val) {
		//std::exchange isn't constexpr
		other.val = 0;
	}
	constexpr snowflake(const snowflake& other) = default;
};

constexpr bool operator!=(const snowflake& a,const snowflake& b) {
	return a.val != b.val;
}

constexpr bool operator<(const snowflake& a,const snowflake& b) {
	return a.val < b.val;
}

constexpr bool operator>(const snowflake& a, const snowflake& b) {
	return a.val > b.val;
}

constexpr bool operator<=(const snowflake& a, const snowflake& b) {
	return a.val <= b.val;
}

constexpr bool operator>=(const snowflake& a, const snowflake& b) {
	return a.val >= b.val;
}

inline void to_json(nlohmann::json& json,const snowflake& wawt) {
	char buffer[20] = {};
	const auto r = std::to_chars(buffer, buffer + 20, wawt.val);
	json = std::string(buffer, r.ptr);
}

inline void from_json(const nlohmann::json& in,snowflake& out) {
	if (in.is_null()) return;
	try{
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

struct id_equal_to{
	explicit id_equal_to(snowflake i) : id(std::move(i)) {}
	template<typename T>
	constexpr bool operator()(T&& thing){
		return thing.id() == id;
	}
	const snowflake id;
};

//this is like a map_view ;-;, i should just make one, then alias this
template<typename T>
struct discord_obj_map{
	discord_obj_map() = default;
	discord_obj_map(const rename_later_4<snowflake,T>& data):m_data(&data){}
	
	template<typename it>
	struct cursor {
		cursor() = default;
		cursor(it i) :m_it(i) {}

		const T& read() const noexcept {
			return *m_it;
		}

		void next()noexcept {
			++m_it;
		}

		bool operator==(const cursor& other)const {
			return m_it == other.m_it;
		}

		it m_it;
	};

	using iterator = iterator_facade<cursor<typename rename_later_4<snowflake, T>::const_iterator>>;
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

	bool contains(snowflake s)const noexcept {
		return m_data->find(s) != m_data->end();
	}

	size_t size()const noexcept {
		return m_data->size();
	}
	bool empty()const noexcept {
		return size() == 0;
	}
private:
	const rename_later_4<snowflake, T>* m_data;
};


//just use auto
template<typename T,typename snowflake_range>
struct discord_obj_list{
	discord_obj_list() = default;
	explicit discord_obj_list(discord_obj_map<T> a, snowflake_range b):
		m_map(a),
		m_keys(std::move(b)){};
	using underlying_iterator = decltype(std::declval<std::add_const_t<snowflake_range>>().begin());


	template<typename value_type_,ptrdiff_t stride>
	struct cursor {
		cursor() = default;
		explicit cursor(discord_obj_list* t_parent, const underlying_iterator t_it) :
			m_parent(t_parent),
			m_it(t_it) {}

		void next()noexcept {
			m_it += stride;
		}

		void prev()noexcept {
			m_it -= stride;
		}

		std::add_const_t<value_type_>& read() const noexcept{
			return m_parent->m_map.at(*m_it);
		}

		ptrdiff_t distance_to(const cursor& other)const noexcept{
			return m_it - other.m_it;
		}

		void advance(ptrdiff_t i)noexcept {
			m_it += i;
		}
		
		template<typename O,int N>
		bool operator==(const cursor<O, N>& other)const {
			return m_it == other.m_it;
		}
	private:
		const discord_obj_list* m_parent;
		underlying_iterator m_it{};
	};

	using iterator = iterator_facade<cursor<T,1>>;
	using const_iterator = iterator_facade<cursor<const T, 1>>;

	using reverse_iterator = iterator_facade<cursor<T, -1>>;
	using const_reverse_iterator = iterator_facade<cursor<const T, -1>>;

	iterator begin() {
		return iterator{ this,const_ids().begin() };
	}
	iterator end() {
		return iterator{ this,const_ids().end() };
	}
	const_iterator begin() const {
		return const_iterator{ this,const_ids().begin() };
	}
	const_iterator end() const{
		return const_iterator{ this,const_ids().end() };
	}
	T& operator[](size_t i){
		return m_map[m_keys[i]];
	}
	const T& operator[](size_t i)const {
		return m_map.at(m_keys[i]);
	}

	size_t size()const noexcept { return m_keys.size(); }

	bool empty()const noexcept {
		return size() == 0;
	}

private:
	std::add_rvalue_reference_t<std::add_const_t<snowflake_range>> const_ids()const noexcept{
		return m_keys;
	}
	discord_obj_map<T> m_map;
	snowflake_range m_keys {};
};

static inline constexpr auto transform_to_pair = [](auto&& a, auto&& fn1,auto&& fn2){
	//evaluation order is undefined ;-;, so i can't inline this
	auto ret = std::invoke(std::forward<decltype(fn1)>(fn1), a);
	auto other = std::invoke(std::forward<decltype(fn2)>(fn2), ret);
	return std::make_pair(std::move(ret), std::move(other));
};

template<typename T>
std::pair<snowflake,T> get_return_id(const nlohmann::json& json) {
	auto ret = json.get<T>();
	const snowflake id = ret.id();
	return { id,std::move(ret) };
}

static inline const auto id_comp = [](auto&& a, snowflake b) {return a.id() < b; };

static inline const auto get_id = [](auto&& a) {return a.id(); };
