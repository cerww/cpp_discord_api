#pragma once
#include "crtp_stuff.h"
#include <iterator>
#include <type_traits>
#include <range/v3/core.hpp>

template<typename T,typename = void>
struct readable:std::false_type{};

template<typename T>
struct readable<T,std::enable_if_t<
	!std::is_void_v<decltype(std::declval<T>().read())>>
> :std::true_type {};

template<typename T, typename = void>
struct incrementable :std::false_type {};

template<typename T>
struct incrementable<T, std::void_t<decltype(std::declval<T>().next())>> :std::true_type {};

template<typename T, typename = void>
struct decrementable :std::false_type {};

template<typename T>
struct decrementable<T, std::void_t<decltype(std::declval<T>().prev())>> :std::true_type {};

template<typename T, typename = void>
struct random_access_interface :std::false_type {};

template<typename T>
struct random_access_interface<T, std::void_t<
	decltype(std::declval<T>().distance_to(std::declval<T>())),
	decltype(std::declval<T>().advance(std::declval<T>().distance_to(std::declval<T>())))>
> :std::true_type {};

template<typename T, typename = void>
struct equality_comparable :std::false_type {};

template<typename T>
// ReSharper disable once CppIdenticalOperandsInBinaryExpression
struct equality_comparable<T, std::void_t<decltype(std::declval<T>()==std::declval<T>())>> :std::true_type {};

template<typename T,typename = void>
struct has_done:std::false_type{};

template<typename T>
struct has_done<T,std::enable_if_t<std::is_same_v<bool,decltype(std::declval<T>().done())>>>:std::true_type {};

template<typename T, typename = void>
struct is_contiguous:std::false_type {};

template<typename T>
struct is_contiguous<T, std::enable_if_t<T::is_contiguous>>:std::true_type {};

namespace thingy_detail{
	template<bool b>
	struct conditional{
		template<typename T,typename F>
		using type = T;
	};

	template<>
	struct conditional <false>{
		template<typename T, typename F>
		using type = F;
	};
	template<bool b,typename T,typename F>
	using conditional_t = typename conditional<b>::template type<T, F>;	
}

template<typename cursor>
struct iterator_facade{
	static constexpr bool is_done_boi = has_done<cursor>::value;
	static_assert(incrementable<cursor>::value && readable<cursor>::value && (equality_comparable<cursor>::value || is_done_boi));

	constexpr iterator_facade() = default;

	template<typename... Args,std::enable_if_t<std::is_constructible_v<cursor,Args...>,int> = 0>
	constexpr iterator_facade(Args&&... args):m_base(std::forward<Args>(args)...){}

	using value_type = std::decay_t<decltype(std::declval<cursor>().read())>;
	using difference_type = ptrdiff_t;
	using reference = decltype(std::declval<cursor>().read());
	using iterator_category = thingy_detail::conditional_t<
		equality_comparable<cursor>::value,
		thingy_detail::conditional_t<
			decrementable<cursor>::value, 
			thingy_detail::conditional_t<
				random_access_interface<cursor>::value,
				std::random_access_iterator_tag,
				std::bidirectional_iterator_tag>,
			std::forward_iterator_tag>,
		std::input_iterator_tag>;
private:
	static constexpr bool is_bidi() noexcept{
		return std::is_base_of_v<std::bidirectional_iterator_tag, iterator_category>;
	}

	static constexpr bool is_random_access() noexcept{
		return std::is_base_of_v<std::random_access_iterator_tag, iterator_category>;
	}
public:
	constexpr reference operator*()const& {
		return m_base.read();
	}

	constexpr reference operator*()& {
		return m_base.read();
	}

	constexpr reference operator*()const&& {
		return m_base.read();
	}

	constexpr reference operator*() && {
		return m_base.read();
	}

	template<typename B = int, std::enable_if_t<std::is_same_v<B, int> && std::is_reference_v<reference>, int> = 0>
	constexpr std::remove_reference_t<reference>* operator->() & {
		return &m_base.read();
	}

	template<typename B = int, std::enable_if_t<std::is_same_v<B, int> && std::is_reference_v<reference>, int> = 0>
	constexpr std::remove_reference_t<reference>* operator->() const& {
		return &m_base.read();
	}

	template<typename B = int, std::enable_if_t<std::is_same_v<B, int> && std::is_reference_v<reference>, int> = 0>
	constexpr std::remove_reference_t<reference>* operator->() && {
		return &m_base.read();
	}

	template<typename B = int, std::enable_if_t<std::is_same_v<B, int> && std::is_reference_v<reference>, int> = 0>
	constexpr std::remove_reference_t<reference>* operator->() const&& {
		return &m_base.read();
	}

	constexpr iterator_facade& operator++() {
		m_base.next();
		return *this;
	}

	constexpr iterator_facade operator++(int) {
		auto t = *this;
		m_base.next();
		return t;
	}

	template<typename B = int>
	constexpr std::enable_if_t<is_bidi() && std::is_same_v<B, int>, iterator_facade&> operator--(){
		m_base.prev();
		return *this;
	}

	template<typename B = int>
	constexpr std::enable_if_t<is_bidi() && std::is_same_v<B, int>, iterator_facade> operator--(int) {
		auto t = *this;
		m_base.prev();
		return t;
	}

	template<typename B = int>
	constexpr std::enable_if_t<is_random_access() && std::is_same_v<B, int>, iterator_facade> operator+(difference_type a) const{
		auto o = *this;
		o.m_base.advance(a);
		return o;
	}

	template<typename B = int>
	friend constexpr std::enable_if_t<is_random_access() && std::is_same_v<B, int>, iterator_facade> operator+(difference_type a,const iterator_facade& me) {
		auto o = me;
		o.m_base.advance(a);
		return o;
	}

	template<typename B = int>
	constexpr std::enable_if_t<is_random_access() && std::is_same_v<B, int>, iterator_facade> operator-(difference_type a) const{
		auto o = *this;
		o.m_base.advance(-1 * a);
		return o;
	}

	template<typename B = int>
	constexpr std::enable_if_t<is_random_access() && std::is_same_v<B, int>, difference_type> operator-(const iterator_facade& other)const {
		return m_base.distance_to(other.m_base);
	}

	template<typename B = int>
	constexpr std::enable_if_t<is_random_access() && std::is_same_v<B, int>, iterator_facade>& operator+=(difference_type n) {
		m_base.advance(n);
		return *this;
	}

	template<typename B = int>
	constexpr std::enable_if_t<is_random_access() && std::is_same_v<B, int>, iterator_facade>& operator-=(difference_type n) {
		m_base.advance(-1* n);
		return *this;
	}

	template<typename B = int>
	constexpr std::enable_if_t<is_random_access() && std::is_same_v<B, int>, reference> operator[](difference_type n) {
		auto t = m_base;
		t.advance(n);
		return t.read();
	}

	template<typename B = int>
	constexpr std::enable_if_t<is_random_access() && std::is_same_v<B, int>, bool> operator<(const iterator_facade& other) const{
		return m_base.distance_to(other.m_base) < 0;
	}

	template<typename B = int>
	constexpr std::enable_if_t<is_random_access() && std::is_same_v<B, int>, bool> operator>(const iterator_facade& other)const {
		return m_base.distance_to(other.m_base) > 0;
	}

	template<typename B = int>
	constexpr std::enable_if_t<is_random_access() && std::is_same_v<B, int>, bool> operator<=(const iterator_facade& other)const {
		return m_base.distance_to(other.m_base) <= 0;
	}

	template<typename B = int>
	constexpr std::enable_if_t<is_random_access() && std::is_same_v<B, int>, bool> operator>=(const iterator_facade& other)const {
		return m_base.distance_to(other.m_base) >= 0;
	}

	template<typename B = int>
	constexpr std::enable_if_t<std::is_same_v<B, int> && equality_comparable<cursor>::value, bool> operator==(const iterator_facade& other) const{
		return m_base == other.m_base;
	}

	template<typename B = int>
	constexpr std::enable_if_t<std::is_same_v<B, int> && equality_comparable<cursor>::value, int> operator!=(const iterator_facade& other) const{
		return !(m_base == other.m_base);
	}
	
	template<typename B = int>
	constexpr std::enable_if_t<std::is_same_v<B, int> && is_done_boi, bool> operator!=(ranges::default_sentinel) const {
		return m_base.done();
	}

	template<typename B = int>
	constexpr std::enable_if_t<std::is_same_v<B, int> && is_done_boi, bool> operator==(ranges::default_sentinel) const {
		return !m_base.done();
	}

	constexpr cursor& base() noexcept{
		return m_base;
	}

	constexpr const cursor& base() const noexcept {
		return m_base;
	}
private:
	cursor m_base{};
};










