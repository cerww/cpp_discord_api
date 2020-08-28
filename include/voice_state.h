#pragma once
#include <nlohmann/json.hpp>
#include "snowflake.h"
#include "guild_member.h"
#include "../common/common/optional_from_json.h"

struct voice_state {
	snowflake channel_id() const noexcept { return m_channel_id; }

	snowflake user_id() const noexcept { return m_user_id; }

	std::string_view session_id() const noexcept { return m_session_id; }

	bool deaf() const noexcept { return m_deaf; }

	bool mute() const noexcept { return m_mute; }

	bool self_deaf() const noexcept { return m_self_deaf; }

	bool self_mute() const noexcept { return m_self_mute; }

	bool suppress() const noexcept { return m_suppress; }

	const std::optional<guild_member>& member() const { return m_member; }

private:
	snowflake m_channel_id;
	snowflake m_user_id;
	std::string m_session_id;
	bool m_deaf = false;
	bool m_mute = false;
	bool m_self_deaf = false;
	bool m_self_mute = false;
	bool m_suppress = false;
	std::optional<guild_member> m_member;

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
	vs.m_channel_id = json["channel_id"].get<snowflake>();
	vs.m_user_id = json["user_id"].get<snowflake>();
	vs.m_session_id = json["session_id"].get<std::string>();
	vs.m_deaf = json["deaf"].get<bool>();
	vs.m_mute = json["mute"].get<bool>();
	vs.m_self_deaf = json["self_deaf"].get<bool>();
	vs.m_self_mute = json["self_mute"].get<bool>();
	vs.m_suppress = json["suppress"].get<bool>();
	vs.m_member = json.value("member", std::optional<guild_member>());
}

inline void from_json(const nlohmann::json& json, voice_state2& vs) {
	from_json(json, static_cast<voice_state&>(vs));
	vs.m_guild_id = json["guild_id"].get<snowflake>();
}

struct voice_region {
	std::string_view id() const noexcept { return m_id; }

	std::string_view name() const noexcept { return m_name; }

	bool vip() const noexcept { return m_vip; }

	bool optimal() const noexcept { return m_optimal; }

	bool depricated() const noexcept { return m_depricated; }

	bool custom() const noexcept { return m_custom; }

private:
	std::string m_id{};
	std::string m_name{};
	bool m_vip = false;
	bool m_optimal = false;
	bool m_depricated = false;
	bool m_custom = false;
	friend void from_json(const nlohmann::json&, voice_region&);
};

inline void from_json(const nlohmann::json& json, voice_region& r) {
	r.m_id = json["id"].get<std::string>();
	r.m_name = json["name"].get<std::string>();
	r.m_vip = json["vip"].get<bool>();
	r.m_optimal = json["optimal"].get<bool>();
	r.m_depricated = json["depricated"].get<bool>();
	r.m_custom = json["custom"].get<bool>();
}
