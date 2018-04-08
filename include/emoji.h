#pragma once
#include <nlohmann/json.hpp>
#include "snowflake.h"

class partial_emoji{
public:
	snowflake id() const noexcept;
	const std::string& name() const noexcept;
private:
	snowflake m_id;
	std::string m_name;
	friend void from_json(const nlohmann::json&, partial_emoji&);
};

class emoji:public partial_emoji{
public:
	snowflake user_id()const noexcept { return m_user_id; }
	std::vector<snowflake>& roles()noexcept { return m_roles; }
	const std::vector<snowflake>& roles()const noexcept { return m_roles; }
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
