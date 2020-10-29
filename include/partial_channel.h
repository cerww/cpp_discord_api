#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "permision_overwrite.h"

struct internal_shard;

enum class channel_type {
	guild_text,
	dm,
	guild_voice,
	group_dm,
	catagory,	
	news,
	store,
	unknown,
	unknown_guild	
};

struct partial_channel {
	snowflake id() const noexcept { return m_id; }

	std::string_view name() const noexcept { return m_name; }

	channel_type type()const noexcept { return m_type; }
	
private:
	std::string m_name;
	channel_type m_type = channel_type::unknown;
	snowflake m_id;

	friend struct internal_shard;
	friend void from_json(const nlohmann::json& json, partial_channel& channel);
};

inline void to_json(nlohmann::json& json, const partial_channel& ch) {
	json["id"] = ch.id();	
	json["name"] = ch.name();
}

inline void from_json(const nlohmann::json& json, partial_channel& channel) {
	channel.m_id = json["id"].get<snowflake>();
	channel.m_name = json.value("name", std::string());
	channel.m_type = json.value("type", channel_type::unknown);
}
