#pragma once
#include "lol_wat_vector.h"
#include <nlohmann/json.hpp>
#include <range/v3/all.hpp>
#include <folly/FBString.h>

template<typename T>
inline void from_json(const nlohmann::json& json, lol_wat_vector<T>& out) {
	const auto size = json.size();
	if (size) {
		out.resize(size);
		ranges::copy(json | ranges::views::transform(&nlohmann::json::get<T>), out.begin());
	}
}


template<typename T>
inline void to_json(nlohmann::json& json, const lol_wat_vector<T>& out) {	
	for(const T& a:out) {
		json.push_back(a);
	}
}

inline void from_json(const nlohmann::json& json, folly::fbstring& out) {
	out = json.get_ref<const std::string&>();
}

inline void to_json(nlohmann::json& json, const folly::fbstring& str) {
	json = str | ranges::to<std::string>();
}

