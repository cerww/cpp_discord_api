#pragma once
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "permission.h"
#include "snowflake.h"

namespace cacheless {

enum class overwrite_type {
	role,
	member
};

template<typename str_t>
overwrite_type string_to_overwrite_type(str_t&& str) {
	if (str == "role")
		return overwrite_type::role;
	if (str == "member")
		return overwrite_type::member;
	throw std::runtime_error("invalid str");
}

std::string overwrite_type_to_string(const overwrite_type e);

struct permission_overwrite {
	snowflake id;
	overwrite_type type = overwrite_type::role;
	size_t allow = 0;
	size_t deny = 0;
	friend void to_json(nlohmann::json& json, const permission_overwrite& data);
	friend void from_json(const nlohmann::json& json, permission_overwrite& data);
};

void to_json(nlohmann::json& json, const permission_overwrite& data);

void from_json(const nlohmann::json& json, permission_overwrite& data);
}
