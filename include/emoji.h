#pragma once
#include <nlohmann/json.hpp>
#include "snowflake.h"
#include <range/v3/core.hpp>

struct partial_emoji{
	snowflake id() const noexcept;
	std::string_view name() const noexcept;
private:
	snowflake m_id;
	std::string m_name;
	friend void from_json(const nlohmann::json&, partial_emoji&);
};

struct emoji:partial_emoji{
	snowflake user_id()const noexcept { return m_user_id; }	
	auto roles()const noexcept {
		return m_roles | ranges::views::all;
	}
private:
	std::vector<snowflake> m_roles;
	snowflake m_user_id;
};

inline void from_json(const nlohmann::json& json,partial_emoji& e) {
	e.m_id = json["id"].get<snowflake>();
	e.m_name = json["name"].get<std::string>();
}

inline void from_json(const nlohmann::json& json, emoji& e) {
	from_json(json, static_cast<partial_emoji&>(e));
}
