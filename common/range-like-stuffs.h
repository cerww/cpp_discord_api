#pragma once
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
