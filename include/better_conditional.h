#pragma once

template<bool condition>
struct better_conditional {
	template<typename T, typename F>
	using value = T;
};

template<>
struct better_conditional<false> {
	template<typename T, typename F>
	using value = F;
};

template<bool condition, typename T, typename F>
using better_conditional_t = typename better_conditional<condition>::template value<T, F>;
