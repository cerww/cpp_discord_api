#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "snowflake.h"
#include "permission.h"


class Role{
public:
	snowflake id() const noexcept;;
	int position() const noexcept;
	permission permissions() const noexcept;
	bool managed() const noexcept;
	bool mentionable() const noexcept;
	bool hoist() const noexcept;
	const std::string& name() const noexcept;

private:
	snowflake m_id;
	std::string m_name;
	int m_position = 0;
	permission m_permissions;
	bool m_managed = false;
	bool m_mentionable = false;
	bool m_hoist = false;
	friend void from_json(const nlohmann::json& json, Role& other);
};

void to_json(nlohmann::json& json, const Role& r);

void from_json(const nlohmann::json& json, Role& other);


