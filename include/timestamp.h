#pragma once
#include <nlohmann/json.hpp>

struct timestamp{
	int i = 0;
};

inline void from_json(const nlohmann::json&, timestamp& t) {
	
}
