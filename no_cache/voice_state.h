#pragma once
#include <nlohmann/json.hpp>
#include "snowflake.h"
#include "guild_member.h"

namespace cacheless {

struct voice_state {
	snowflake channel_id;
	snowflake user_id;
	std::string session_id;
	bool deaf = false;
	bool mute = false;
	bool self_deaf = false;
	bool self_mute = false;
	bool suppress = false;
	std::optional<guild_member> member;
	std::optional<snowflake> guild_id;
	
	
	friend struct internal_shard;
	friend void from_json(const nlohmann::json& json, voice_state& vs);
};

struct voice_state2 :voice_state {
	snowflake guild_id() const noexcept { return m_guild_id; }

private:
	snowflake m_guild_id;
	friend void from_json(const nlohmann::json& json, voice_state2& vs);
};

inline void from_json(const nlohmann::json& json, voice_state& vs) {
	vs.channel_id = json["channel_id"].get<snowflake>();
	vs.user_id = json["user_id"].get<snowflake>();
	vs.session_id = json["session_id"].get<std::string>();
	vs.deaf = json["deaf"].get<bool>();
	vs.mute = json["mute"].get<bool>();
	vs.self_deaf = json["self_deaf"].get<bool>();
	vs.self_mute = json["self_mute"].get<bool>();
	vs.suppress = json["suppress"].get<bool>();
	vs.member = json.value("member", std::optional<guild_member>());
	if(json.contains("guild_id")) {
		vs.guild_id = json["guild_id"].get<snowflake>();
	}
}

inline void from_json(const nlohmann::json& json, voice_state2& vs) {
	from_json(json, static_cast<voice_state&>(vs));
	vs.m_guild_id = json["guild_id"].get<snowflake>();
}

struct voice_region {
	std::string id{};
	std::string name{};
	bool vip = false;
	bool optimal = false;
	bool depricated = false;
	bool custom = false;
	friend void from_json(const nlohmann::json&, voice_region&);
};

inline void from_json(const nlohmann::json& json, voice_region& r) {
	r.id = json["id"].get<std::string>();
	r.name = json["name"].get<std::string>();
	r.vip = json["vip"].get<bool>();
	r.optimal = json["optimal"].get<bool>();
	r.depricated = json["depricated"].get<bool>();
	r.custom = json["custom"].get<bool>();
}
};
