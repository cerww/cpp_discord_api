#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "snowflake.h"
#include "permission.h"

namespace cacheless {
struct guild_role {
	std::string to_mentionable_string() const {
		return "<@&" + std::to_string(id.val) + ">";
	}

	snowflake id;
	std::string name;
	int position = 0;
	permission permissions;
	bool managed = false;
	bool mentionable = false;
	bool hoist = false;
	friend void from_json(const nlohmann::json& json, guild_role& other);
};

void to_json(nlohmann::json& json, const guild_role& r);

void from_json(const nlohmann::json& json, guild_role& other);

}
