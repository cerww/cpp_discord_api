#pragma once
#include <nlohmann/json.hpp>

struct embed { };

inline void from_json(const nlohmann::json&, embed&) { }

inline void to_json(nlohmann::json&,const embed&) {
	
}