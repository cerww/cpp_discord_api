#pragma once
#include <nlohmann/json.hpp>
#include "snowflake.h"

struct permissions {
	//not an enum so i can still use |, &
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
	permission() = default;
	explicit permission(size_t t):m_permission(t){}
	permission& add_permissions(size_t p) {
		m_permission |= p;
		return *this;
	};

	permission& add_permissions(permission p) {
		m_permission |= p.data();
		return *this;
	};
	
	permission& remove_permissions(size_t p) {
		m_permission &= ~p;
		return *this;
	};

	permission& remove_permissions(permission p) {
		m_permission &= ~p.data();
		return *this;
	};

	permission& combine_permissions(permission other) {
		m_permission |= other.m_permission;
		return *this;
	}

	permission intersection(permission other) const noexcept{
		return *this & other;
	}

	bool has_permission(size_t p) const noexcept{
		return (m_permission & p) == p;
	};

	bool has_permission(permission p) const noexcept {
		return (m_permission & p.data()) == p.data();
	};

	permission operator|(permission other) const noexcept{
		return other.combine_permissions(*this);
	}

	permission operator&(permission other) const noexcept {
		return permission(m_permission & other.m_permission);
	}

	size_t data()const noexcept { return m_permission; }
private:
	size_t m_permission = 0;
};

inline void to_json(nlohmann::json& json, const permission& p) {
	json = p.data();
};

inline void from_json(const nlohmann::json& json, permission& p) {
	if (json.is_null())return;
	p = permission(json.get<size_t>());
}

