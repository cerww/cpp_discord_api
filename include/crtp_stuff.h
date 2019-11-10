#pragma once

template<typename T>
struct crtp {
	constexpr T& self() { return static_cast<T&>(*this); }

	constexpr T const& self() const { return static_cast<T const&>(*this); }
};
