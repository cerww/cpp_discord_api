#pragma once
#include "crtp_stuff.h"
#include "arrow_proxy.h"
#include <iterator>
#include <type_traits>
#include "better_conditional.h"
#include <range/v3/core.hpp>

// clang-format off
namespace detail_iterator_facade {
	template<typename T, typename = void>
	struct readable :std::false_type {};

	template<typename T>
	struct readable<T, std::enable_if_t<
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
	struct equality_comparable<T, std::void_t<decltype(std::declval<T>() == std::declval<T>())>> :std::true_type {};

	template<typename T, typename = void>
	struct has_done :std::false_type {};

	template<typename T>
	struct has_done<T, std::enable_if_t<std::is_same_v<bool, decltype(std::declval<T>().done())>>> :std::true_type {};

	template<typename T, typename = void>
	struct is_contiguous :std::false_type {};

	template<typename T>
	struct is_contiguous<T, std::enable_if_t<T::is_contiguous>> :std::true_type {};
};

template<typename cursor>
struct iterator_facade {
	static constexpr bool is_is_done_iterator = detail_iterator_facade::has_done<cursor>::value;
	static_assert(detail_iterator_facade::incrementable<cursor>::value&& detail_iterator_facade::readable<cursor>::value && (detail_iterator_facade::equality_comparable<cursor>::value || is_is_done_iterator));

	constexpr iterator_facade() = default;

	template<typename... Args, std::enable_if_t<std::is_constructible_v<cursor, Args...>, int> = 0>
	constexpr iterator_facade(Args && ... args) :m_base(std::forward<Args>(args)...) {}

	using reference = decltype(std::declval<cursor>().read());
	using value_type = std::remove_cvref_t<reference>;
	using difference_type = ptrdiff_t;
	using iterator_category =
		better_conditional_t<
		detail_iterator_facade::equality_comparable<cursor>::value,
		better_conditional_t <
		detail_iterator_facade::decrementable<cursor>::value,
		better_conditional_t<
		detail_iterator_facade::random_access_interface<cursor>::value,
		std::random_access_iterator_tag,
		std::bidirectional_iterator_tag>,
		std::forward_iterator_tag>,
		std::input_iterator_tag>;

	using pointer = better_conditional_t<std::is_reference_v<reference>, std::add_pointer_t<value_type>, arrow_proxy<reference>>;
private:
	static constexpr bool is_random_access = std::is_base_of_v<std::random_access_iterator_tag, iterator_category>;

	static constexpr bool is_bidi = std::is_base_of_v<std::bidirectional_iterator_tag, iterator_category>;
	
public:
	constexpr reference operator*()const & {
		return m_base.read();
	}

	constexpr reference operator*() & {
		return m_base.read();
	}

	constexpr reference operator*()const && {
		return m_base.read();
	}

	constexpr reference operator*() && {
		return m_base.read();
	}

	//template<typename B = int, std::enable_if_t<std::is_same_v<B, int>&& std::is_reference_v<reference>, int> = 0>
	constexpr std::remove_reference_t<reference>* operator->() & requires std::is_reference_v<reference>{
		return &m_base.read();
	}

	//template<typename B = int, std::enable_if_t<std::is_same_v<B, int>&& std::is_reference_v<reference>, int> = 0>
	constexpr std::remove_reference_t<reference>* operator->() const & requires std::is_reference_v<reference>{
		return &m_base.read();
	}

	//template<typename B = int, std::enable_if_t<std::is_same_v<B, int>&& std::is_reference_v<reference>, int> = 0>
	constexpr std::remove_reference_t<reference>* operator->() && requires std::is_reference_v<reference> {
		return &m_base.read();
	}

	//template<typename B = int, std::enable_if_t<std::is_same_v<B, int>&& std::is_reference_v<reference>, int> = 0>	
	constexpr std::remove_reference_t<reference>* operator->() const && requires std::is_reference_v<reference>{
		return &m_base.read();
	}

	constexpr arrow_proxy<reference> operator->() & requires !std::is_reference_v<reference>{
		return arrow_proxy<reference>{m_base.read()};
	}

	constexpr arrow_proxy<reference> operator->() const & requires !std::is_reference_v<reference>{
		return arrow_proxy<reference>{m_base.read()};
	}

	constexpr arrow_proxy<reference> operator->() && requires !std::is_reference_v<reference>{
		return arrow_proxy<reference>{m_base.read()};
	}

	constexpr arrow_proxy<reference> operator->() const && requires !std::is_reference_v<reference>{
		return arrow_proxy<reference>{m_base.read()};
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

	constexpr iterator_facade& operator--() requires is_bidi{
		m_base.prev();
		return *this;
	}

	constexpr iterator_facade operator--(int)requires is_bidi{
		auto t = *this;
		m_base.prev();
		return t;
	}
	
	constexpr iterator_facade operator+(difference_type a) const requires is_random_access{
		auto o = *this;
		o.m_base.advance(a);
		return o;
	}

	friend constexpr iterator_facade operator+(difference_type a, const iterator_facade& me) requires is_random_access{
		auto o = me;
		o.m_base.advance(a);
		return o;
	}

	constexpr iterator_facade operator-(difference_type a) const requires is_random_access{
		auto o = *this;
		o.m_base.advance(-1 * a);
		return o;
	}

	constexpr difference_type operator-(const iterator_facade & other)const requires is_random_access{
		return m_base.distance_to(other.m_base);
	}

	constexpr iterator_facade& operator+=(difference_type n) requires is_random_access{
		m_base.advance(n);
		return *this;
	}

	constexpr iterator_facade& operator-=(difference_type n)requires is_random_access{
		m_base.advance(-1 * n);
		return *this;
	}

	
	constexpr reference operator[](difference_type n) requires is_random_access{
		auto t = m_base;
		t.advance(n);
		return t.read();
	}

	constexpr bool operator<(const iterator_facade & other) const requires is_random_access{
		return m_base.distance_to(other.m_base) < 0;
	}

	constexpr bool operator>(const iterator_facade & other)const requires is_random_access{
		return m_base.distance_to(other.m_base) > 0;
	}

	constexpr bool operator<=(const iterator_facade & other)const requires is_random_access{
		return m_base.distance_to(other.m_base) <= 0;
	}

	constexpr bool operator>=(const iterator_facade & other)const requires is_random_access{
		return m_base.distance_to(other.m_base) >= 0;
	}

	constexpr bool operator==(const iterator_facade & other) const requires detail_iterator_facade::equality_comparable<cursor>::value{
		return m_base == other.m_base;
	}

	constexpr bool operator!=(const iterator_facade & other) const requires detail_iterator_facade::equality_comparable<cursor>::value{
		return !(m_base == other.m_base);
	}

	constexpr bool operator!=(ranges::default_sentinel_t) const requires is_is_done_iterator{
		return !m_base.done();
	}

	constexpr bool operator==(ranges::default_sentinel_t) const requires is_is_done_iterator{
		return m_base.done();
	}

	constexpr friend bool operator!=(ranges::default_sentinel_t, const iterator_facade & me) requires is_is_done_iterator{
		return !me.m_base.done();
	}

	constexpr friend bool operator==(ranges::default_sentinel_t, const iterator_facade & me)requires is_is_done_iterator{
		return me.m_base.done();
	}

	constexpr cursor& base() noexcept {
		return m_base;
	}

	constexpr const cursor& base() const noexcept {
		return m_base;
	}

	template<typename other_cursor_t> requires std::is_convertible_v<cursor, other_cursor_t> && !std::is_same_v<cursor, other_cursor_t>
	operator iterator_facade<other_cursor_t>() const {
		return iterator_facade<other_cursor_t>(other_cursor_t(m_base));
	}
private:
	cursor m_base{};
};



// clang-format on

