#pragma once
#include <type_traits>
#include <utility>

template<typename map_t, typename U, typename fn>
bool insert_proj_as_key(map_t& m, U&& i, fn&& proj) {
	return m.insert(std::make_pair(std::invoke(proj, std::forward<U>(i)), std::forward<U>(i))).second;
}



