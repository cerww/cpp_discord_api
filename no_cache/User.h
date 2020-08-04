#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include "discord_enums.h"
#include "snowflake.h"
#include <optional>

namespace cacheless {

struct user {
	snowflake id;
	std::string username = "";
	std::string avatar;
	int16_t discriminator = 0;
	bool bot = false;
	/*
	system?			boolean
	mfa_enabled?	boolean
	locale?			string
	verified?		boolean
	email?			?string
	flags?			integer
	premium_type?	int, 0,1,2
	public_flags?	int
	*/


	friend struct internal_shard;
	friend void from_json(const nlohmann::json&, user& other);
};

constexpr int aasdasdasd = sizeof(user);

void to_json(nlohmann::json& json, const user& other);

void from_json(const nlohmann::json& json, user& other);
}
