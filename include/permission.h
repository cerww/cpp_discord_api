#pragma once
#include <nlohmann/json.hpp>

//needs to be own class so i can use | and & and add functions to it
struct permission {
	constexpr permission() = default;
	constexpr explicit permission(const size_t t):m_permission(t){}

	constexpr permission& add_permissions(size_t p) {
		m_permission |= p;
		return *this;
	};

	constexpr permission& add_permissions(permission p) {
		m_permission |= p.data();
		return *this;
	};
	
	constexpr permission& remove_permissions(size_t p) {
		m_permission &= ~p;
		return *this;
	};

	constexpr permission& remove_permissions(permission p) {
		m_permission &= ~p.data();
		return *this;
	};

	constexpr permission& combine_permissions(permission other) {
		m_permission |= other.m_permission;
		return *this;
	}

	constexpr permission intersection(permission other) const noexcept{
		return *this & other;
	}

	constexpr bool has_permission(size_t p) const noexcept{
		return (m_permission & p) == p;
	};

	constexpr bool has_permission(permission p) const noexcept {
		return (m_permission & p.data()) == p.data();
	};

	constexpr permission operator|(permission other) const noexcept{
		return other.combine_permissions(*this);
	}

	constexpr permission operator&(permission other) const noexcept {
		return permission(m_permission & other.m_permission);
	}

	constexpr size_t data()const noexcept { return m_permission; }
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

struct permissions {
	static constexpr permission CREATE_INSTANT_INVITE	= permission(1);
	static constexpr permission KICK_MEMBERS			= permission(2);
	static constexpr permission BAN_MEMBERS				= permission(4);
	static constexpr permission ADMINISTRATOR			= permission(8);
	static constexpr permission MANAGE_CHANNELS			= permission(16);
	static constexpr permission MANAGE_GUILD			= permission(32);
	static constexpr permission ADD_REACTION			= permission(64);
	static constexpr permission VIEW_AUDIT_LOG			= permission(128);
	static constexpr permission VIEW_CHANNEL			= permission(1024);
	static constexpr permission SEND_MESSAGES			= permission(2048);
	static constexpr permission SEND_TTS_MESSAGES		= permission(4096);
	static constexpr permission MANAGE_MESSAGES			= permission(8192);
	static constexpr permission EMBED_LINKS				= permission(16384);
	static constexpr permission ATTACH_FILES			= permission(32768);
	static constexpr permission READ_MESSAGE_HISTORY	= permission(65536);
	static constexpr permission MENTION_EVERYONE		= permission(131072);
	static constexpr permission USE_EXTERNAL_EMOJIS		= permission(262144);
	static constexpr permission CONNECT					= permission(1048576);
	static constexpr permission SPEAK					= permission(2097152);
	static constexpr permission MUTE_MEMBERS			= permission(4194304);
	static constexpr permission DEAFEN_MEMBERS			= permission(8388608);
	static constexpr permission MOVE_MEMBERS			= permission(16777216);
	static constexpr permission USE_VAD					= permission(33554432);
	static constexpr permission CHANGE_NICKNAME			= permission(67108864);
	static constexpr permission MANAGE_NICKNAMES		= permission(134217728);
	static constexpr permission MANAGE_ROLES			= permission(268435456);
	static constexpr permission MANAGE_WEBHOOKS			= permission(536870912);
	static constexpr permission MANAGE_EMOJIS			= permission(1073741824);
};