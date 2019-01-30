#pragma once
#include <nlohmann/json.hpp>
#include "snowflake.h"

struct permissions {
	static constexpr size_t CREATE_INSTANT_INVITE = 1;
	static constexpr size_t KICK_MEMBERS = 2;
	static constexpr size_t BAN_MEMBERS = 4;
	static constexpr size_t ADMINISTRATOR = 8;
	static constexpr size_t MANAGE_CHANNELS = 16;
	static constexpr size_t MANAGE_GUILD = 32;
	static constexpr size_t ADD_REACTION = 64;
	static constexpr size_t VIEW_AUDIT_LOG = 128;
	static constexpr size_t VIEW_CHANNEL = 1024;
	static constexpr size_t SEND_MESSAGES = 2048;
	static constexpr size_t SEND_TTS_MESSAGES = 4096;
	static constexpr size_t MANAGE_MESSAGES = 8192;
	static constexpr size_t EMBED_LINKS = 16384;
	static constexpr size_t ATTACH_FILES = 32768;
	static constexpr size_t READ_MESSAGE_HISTORY = 65536;
	static constexpr size_t MENTION_EVERYONE = 131072;
	static constexpr size_t USE_EXTERNAL_EMOJIS = 262144;
	static constexpr size_t CONNECT = 1048576;
	static constexpr size_t SPEAK = 2097152;
	static constexpr size_t MUTE_MEMBERS = 4194304;
	static constexpr size_t DEAFEN_MEMBERS = 8388608;
	static constexpr size_t MOVE_MEMBERS = 16777216;
	static constexpr size_t USE_VAD = 33554432;
	static constexpr size_t CHANGE_NICKNAME = 67108864;
	static constexpr size_t MANAGE_NICKNAMES = 134217728;
	static constexpr size_t MANAGE_ROLES = 268435456;
	static constexpr size_t MANAGE_WEBHOOKS = 536870912;
	static constexpr size_t MANAGE_EMOJIS = 1073741824;
};

struct permission {
	permission(size_t t):m_permission(t){}
	permission() = default;
	void addPermissions(size_t p) {
		m_permission |= p;
	};
	void combine_permissions(permission other) {
		m_permission |= other.m_permission;
	}
	void removePermissions(size_t p) {
		m_permission &= ~p;
	};
	bool hasPermission(size_t p) const noexcept{
		return (m_permission & p) == p;
	};
	size_t& data() noexcept { return m_permission; }
	const size_t& data()const noexcept { return m_permission; }
private:
	size_t m_permission = 0;
};

inline void to_json(nlohmann::json& json, const permission& p) {
	json = p.data();
};

inline void from_json(const nlohmann::json& json, permission& p) {
	if (json.is_null())return;
	p.data() = json.get<size_t>();
}

