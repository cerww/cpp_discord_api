#pragma once
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "permission.h"
#include "snowflake.h"

enum class overwrite_type {
	role,
	member
};

template <typename str_t>
overwrite_type string_to_overwrite_type(str_t&& str) {
	if (str == "role") return overwrite_type::role;
	if (str == "member") return overwrite_type::member;
	throw std::runtime_error("invalid str");
}

std::string overwrite_type_to_string(const overwrite_type e);

struct permission_overwrite {
	/// @brief id of either role, or member, depending on the type
	/// @return 
	snowflake id() const noexcept { return m_id; }
	overwrite_type type() const noexcept { return m_type; }
	size_t allow() const noexcept{ return m_allow; }
	size_t deny()const noexcept { return m_deny; }
private:
	snowflake m_id;
	overwrite_type m_type = overwrite_type::role;
	size_t m_allow = 0;
	size_t m_deny = 0;
	friend void to_json(nlohmann::json& json, const permission_overwrite& data);
	friend void from_json(const nlohmann::json& json, permission_overwrite& data);
};

void to_json(nlohmann::json& json, const permission_overwrite& data);

void from_json(const nlohmann::json& json, permission_overwrite& data);
