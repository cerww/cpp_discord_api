#pragma once
#include <experimental/generator>
#include <vector>
#include <type_traits>


template<typename rng>
using range_type = decltype(*(std::declval<rng>().begin()));

template<typename rng, typename = void>
struct is_range:std::false_type{};

template<typename rng>
struct is_range<rng, std::void_t<
	decltype(std::declval<rng>().begin() != std::declval<rng>().end()), 
	std::enable_if_t<!std::is_same_v<range_type<rng>,void>>,
	decltype(++std::declval<rng>().begin())	
>> :std::true_type {};

template<typename rng>
constexpr static bool is_range_v = is_range<rng>::value;

template<typename rng, typename t,typename = void>
struct is_range_of:std::false_type {};

template<typename rng, typename t>
struct is_range_of<rng,t,std::enable_if_t<is_range_v<rng> && std::is_same_v<std::decay_t<range_type<rng>>, t>>>:std::true_type{};

template<typename rng,typename t>
constexpr static bool is_range_of_v = is_range_of<rng, t>::value;

template<typename rng, typename fn>
std::experimental::generator<std::decay_t<range_type<rng>>> filter(rng&& range, fn pred) {
	for (auto&& item : range)
		if (pred(item))
			co_yield item;
}

template<typename T, typename rng1>
std::experimental::generator<T> concat_impl(rng1&& rng) {
	for (auto&& i : rng)
		co_yield i;
}

template<typename T,typename rng1,typename ...rngs>
std::experimental::generator<T> concat_impl(rng1&& rng,rngs&&...Rngs) {
	for (auto&& i : rng)
		co_yield i;
	for (auto&& i : concat_impl<T>(std::forward<rngs>(Rngs)...)) 
		co_yield i;	
}

template<typename rng1>
std::experimental::generator<std::decay_t<range_type<std::decay_t<rng1>>>> concat(rng1&& Rng) {
	using type = std::decay_t<range_type<rng1>>;
	for (auto&& i : Rng)
		co_yield i;
}

template<typename rng1,typename... rngs>
std::experimental::generator<std::decay_t<range_type<std::decay_t<rng1>>>> concat(rng1&& Rng,rngs&&... Rngs) {
	using type = std::decay_t<range_type<rng1>>;
	for (auto&& i : Rng)
		co_yield i;
	for(auto&& i:concat_impl<type>(std::forward<rngs>(Rngs)...)) 
		co_yield i;	
}





