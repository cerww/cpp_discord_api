#pragma once
#include <type_traits>
#include <tuple>

namespace metap {
	template<typename T, template<typename...> typename B>
	struct is_specialzation_of :std::false_type {};

	template<template<typename...> typename B, typename... U>
	struct is_specialzation_of<B<U...>, B> :std::true_type {};

	template<template<typename...> typename B, typename... U>
	struct is_specialzation_of<B<U...>&, B> :std::true_type {};

	template<template<typename...> typename B, typename... U>
	struct is_specialzation_of<const B<U...>&, B> :std::true_type {};

	template<template<typename...> typename B, typename... U>
	struct is_specialzation_of<B<U...>&&, B> :std::true_type {};

	template<template<typename...> typename B, typename... U>
	struct is_specialzation_of<const B<U...>&&, B> :std::true_type {};

	template<typename T, template<typename ...> typename B>
	static constexpr bool is_specialization_of_v = is_specialzation_of<T, B>::value;

	template<typename Fn, typename T>
	struct is_invokable_with_tuple {
		//static_assert(false, "T shuold be a tuple");
	};

	template<typename Fn, typename... Ts>
	struct is_invokable_with_tuple<Fn, std::tuple<Ts...>> :
			std::bool_constant<std::is_invocable_v<Fn, Ts...>> { };

	template<typename Fn, typename Tuple>
	static constexpr bool is_invokable_with_tuple_v = is_invokable_with_tuple<Fn, Tuple>::value;

	namespace tests {
		template<typename T>
		struct a {};

		template<typename T = int>
		struct b {};

		template<typename T, typename U>
		struct two_params {};

		static_assert(is_specialization_of_v<a<int>, a>);
		static_assert(is_specialization_of_v<a<bool>, a>);
		static_assert(is_specialization_of_v<b<>, b>);
		static_assert(!is_specialization_of_v<b<>, a>);
		static_assert(is_specialization_of_v<two_params<int, bool>, two_params>);
		static_assert(!is_specialization_of_v<a<int>, two_params>);
		static_assert(is_specialization_of_v<std::tuple<int, int>, std::tuple>);
		static_assert(is_specialization_of_v<std::tuple<int, int>&, std::tuple>);
		static_assert(is_specialization_of_v<const std::tuple<int, int>&, std::tuple>);
		static_assert(is_specialization_of_v<const std::tuple<int, int, bool&>&, std::tuple>);
	}
}
