#pragma once
#include <optional>
#include <nlohmann/json.hpp>

namespace std {//;-;

template<typename T>
void from_json(const nlohmann::json& json, optional<T>& a) {
	if(json.is_null()) {
		a = std::nullopt;
	}else {
		a = json.get<T>();
	}
}

}
