#pragma once
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "permission.h"
#include "snowflake.h"

enum class overwrite_type {
	member,
	role
};

struct permission_overwrite {
	/// @brief id of either role, or member, depending on the type
	/// @return 
	snowflake id() const noexcept { return m_id; }
	overwrite_type type() const noexcept { return m_type; }
	permission allow() const noexcept{ return m_allow; }
	permission deny()const noexcept { return m_deny; }
private:
	snowflake m_id;
	overwrite_type m_type = overwrite_type::member;
	permission m_allow;
	permission m_deny;
	friend void to_json(nlohmann::json& json, const permission_overwrite& data);
	friend void from_json(const nlohmann::json& json, permission_overwrite& data);
};

void to_json(nlohmann::json& json, const permission_overwrite& data);

void from_json(const nlohmann::json& json, permission_overwrite& data);
