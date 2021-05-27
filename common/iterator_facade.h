#pragma once
#include "crtp_stuff.h"
#include "arrow_proxy.h"
#include <iterator>
#include <type_traits>
#include "better_conditional.h"
#include <range/v3/core.hpp>
#include <concepts>

// clang-format off
namespace detail_iterator_facade {
template<typename T>
concept readable = requires(T t)
{
	{t.read()};
};

template<typename T>
concept incrementable = requires(T t)
{
	{t.next()};
};

template<typename T>
concept decrementable = requires(T t)
{
	{t.prev()};
};

template<typename T>
concept random_access_interface = requires(T t,int a)
{
	{t.advance(a)};
	{t.distance_to(t)}->std::integral;
};

template<typename T>
concept equality_comparable = requires(T t)
{
	{t == t}->std::same_as<bool>;
};

template<typename T>
concept has_done = requires(T t)
{
	{t.done()}->std::same_as<bool>;
};

template<typename T>
concept is_contiguous = requires(T t)
{
	{T::is_contiguous};
};


};

template<typename cursor>
struct iterator_facade:private cursor {
	static constexpr bool is_is_done_iterator = detail_iterator_facade::has_done<cursor>;
	static_assert(detail_iterator_facade::incrementable<cursor> && detail_iterator_facade::readable<cursor> && (detail_iterator_facade::equality_comparable<cursor> || is_is_done_iterator));

	using cursor::cursor;
	

	using reference = decltype(std::declval<cursor>().read());
	using value_type = std::remove_cvref_t<reference>;
	using difference_type = ptrdiff_t;
	using iterator_category =
	better_conditional_t<
		detail_iterator_facade::equality_comparable<cursor>,
		better_conditional_t<
			detail_iterator_facade::decrementable<cursor>,
			better_conditional_t<
				detail_iterator_facade::random_access_interface<cursor>,
				std::random_access_iterator_tag,
				std::bidirectional_iterator_tag>,
			std::forward_iterator_tag>,
		std::input_iterator_tag>;

	using pointer = better_conditional_t<std::is_reference_v<reference>, std::add_pointer_t<value_type>, arrow_proxy<reference>>;
private:
	static constexpr bool is_random_access = std::is_base_of_v<std::random_access_iterator_tag, iterator_category>;

	static constexpr bool is_bidi = std::is_base_of_v<std::bidirectional_iterator_tag, iterator_category>;

public:
	constexpr reference operator*() const & {
		return base().read();
	}

	constexpr reference operator*() & {
		return base().read();
	}

	constexpr reference operator*() const && {
		return base().read();
	}

	constexpr reference operator*() && {
		return base().read();
	}

	constexpr std::remove_reference_t<reference>* operator->() & requires std::is_reference_v<reference> {
		return &base().read();
	}

	constexpr std::remove_reference_t<reference>* operator->() const & requires std::is_reference_v<reference> {
		return &base().read();
	}

	constexpr std::remove_reference_t<reference>* operator->() && requires std::is_reference_v<reference> {
		return &base().read();
	}

	constexpr std::remove_reference_t<reference>* operator->() const && requires std::is_reference_v<reference> {
		return &base().read();
	}

	constexpr arrow_proxy<reference> operator->() & requires !std::is_reference_v<reference> {
		return arrow_proxy<reference>{base().read()};
	}

	constexpr arrow_proxy<reference> operator->() const & requires !std::is_reference_v<reference> {
		return arrow_proxy<reference>{base().read()};
	}

	constexpr arrow_proxy<reference> operator->() && requires !std::is_reference_v<reference> {
		return arrow_proxy<reference>{base().read()};
	}

	constexpr arrow_proxy<reference> operator->() const && requires !std::is_reference_v<reference> {
		return arrow_proxy<reference>{base().read()};
	}

	constexpr iterator_facade& operator++() {
		base().next();
		return *this;
	}

	constexpr iterator_facade operator++(int) {
		auto t = *this;
		base().next();
		return t;
	}

	constexpr iterator_facade& operator--() requires is_bidi {
		base().prev();
		return *this;
	}

	constexpr iterator_facade operator--(int)requires is_bidi {
		auto t = *this;
		base().prev();
		return t;
	}

	constexpr iterator_facade operator+(difference_type a) const requires is_random_access {
		auto o = *this;
		o.base().advance(a);
		return o;
	}

	friend constexpr iterator_facade operator+(difference_type a, const iterator_facade& me) requires is_random_access {
		auto o = me;
		o.base().advance(a);
		return o;
	}

	constexpr iterator_facade operator-(difference_type a) const requires is_random_access {
		auto o = *this;
		o.base().advance(-1 * a);
		return o;
	}

	constexpr difference_type operator-(const iterator_facade& other) const requires is_random_access {
		return base().distance_to(other.base());
	}

	constexpr iterator_facade& operator+=(difference_type n) requires is_random_access {
		base().advance(n);
		return *this;
	}

	constexpr iterator_facade& operator-=(difference_type n)requires is_random_access {
		base().advance(-1 * n);
		return *this;
	}


	constexpr reference operator[](difference_type n) requires is_random_access {
		auto t = base();
		t.advance(n);
		return t.read();
	}

	constexpr bool operator<(const iterator_facade& other) const requires is_random_access {
		return base().distance_to(other.base()) < 0;
	}

	constexpr bool operator>(const iterator_facade& other) const requires is_random_access {
		return base().distance_to(other.base()) > 0;
	}

	constexpr bool operator<=(const iterator_facade& other) const requires is_random_access {
		return base().distance_to(other.base()) <= 0;
	}

	constexpr bool operator>=(const iterator_facade& other) const requires is_random_access {
		return base().distance_to(other.base()) >= 0;
	}

	constexpr bool operator==(const iterator_facade& other) const requires detail_iterator_facade::equality_comparable<cursor>::value {
		return base() == other.base();
	}

	constexpr bool operator!=(const iterator_facade& other) const requires detail_iterator_facade::equality_comparable<cursor>::value {
		return !(base() == other.base());
	}

	constexpr bool operator!=(ranges::default_sentinel_t) const requires is_is_done_iterator {
		return !base().done();
	}

	constexpr bool operator==(ranges::default_sentinel_t) const requires is_is_done_iterator {
		return base().done();
	}

	constexpr friend bool operator!=(ranges::default_sentinel_t, const iterator_facade& me) requires is_is_done_iterator {
		return !me.base().done();
	}

	constexpr friend bool operator==(ranges::default_sentinel_t, const iterator_facade& me)requires is_is_done_iterator {
		return me.base().done();
	}

	constexpr cursor& base() noexcept {
		return static_cast<cursor&>(*this);
	}

	constexpr const cursor& base() const noexcept {
		return static_cast<const cursor&>(*this);
	}

	template<typename other_cursor_t> requires std::is_convertible_v<cursor, other_cursor_t> && !std::is_same_v<cursor, other_cursor_t>
	operator iterator_facade<other_cursor_t>() const {
		return iterator_facade<other_cursor_t>(other_cursor_t(base()));
	}

};


// clang-format on
