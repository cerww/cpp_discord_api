#pragma once
#include <nlohmann/json.hpp>
#include "../common/big_uint.h"

//needs to be own class so i can use | and & and add functions to it
struct permission {
	permission() = default;
	
	explicit permission(const uint64_t t):m_permission_bit_set(t){}

	permission(big_uint t):m_permission_bit_set(std::move(t)){}

	permission& add_permissions(uint64_t p) {
		m_permission_bit_set |= p;
		return *this;
	};

	permission& add_permissions(const big_uint& p) {
		m_permission_bit_set |= p;
		return *this;
	};

	permission& add_permissions(permission p) {
		return add_permissions(p.data());
	};
	
	permission& remove_permissions(const big_uint& p) {
		//m_permission_bit_set &= ~p;
		bitwise_and_but_missing_bits_are_1(std::move(m_permission_bit_set), ~p);
		return *this;
	}

	permission& remove_permissions(const permission& p) {
		//m_permission_bit_set &= ~p.data();
		bitwise_and_but_missing_bits_are_1(std::move(m_permission_bit_set), ~p.data());
		return *this;
	}

	permission& combine_permissions(const permission& other) {
		m_permission_bit_set |= other.m_permission_bit_set;
		return *this;
	}

	permission intersection(const permission& other) const noexcept{
		return *this & other;
	}

	bool has_permission(uint64_t p) const noexcept{
		return (m_permission_bit_set & p) == p;
	}

	bool has_permission(const permission& p) const noexcept {
		return (m_permission_bit_set & p.data()) == p.data();
	}

	permission operator|(permission other) const noexcept{
		return other.combine_permissions(*this);
	}

	permission operator&(const permission& other) const noexcept {
		return permission(m_permission_bit_set & other.m_permission_bit_set);
	}

	permission operator+(permission o) const noexcept{
		return o.add_permissions(*this);
	}

	permission operator-(const permission& o) const noexcept {
		return permission(*this).remove_permissions(o);
	}

	permission& operator+=(const permission& o) noexcept {
		return add_permissions(o);
	}

	permission& operator-=(const permission& o) noexcept {
		return remove_permissions(o);
	}	
	
	const big_uint& data()const noexcept { return m_permission_bit_set; }
private:	
	big_uint m_permission_bit_set;
	friend void from_json(const nlohmann::json&, permission&);
};

inline void to_json(nlohmann::json& json, const permission& p) {
	//json = p.data();
	json = to_string(p.data());
};

inline void from_json(const nlohmann::json& json, permission& p) {
	if (json.is_null()) {
		return;
	}
	//p = permission(json.get<uint64_t>());
	p.m_permission_bit_set = big_uint(json.get<std::string_view>());	
}

struct permissions {
	static const inline permission CREATE_INSTANT_INVITE	= permission(1);
	static const inline permission KICK_MEMBERS				= permission(2);
	static const inline permission BAN_MEMBERS				= permission(4);
	static const inline permission ADMINISTRATOR			= permission(8);
	static const inline permission MANAGE_CHANNELS			= permission(16);
	static const inline permission MANAGE_GUILD				= permission(32);
	static const inline permission ADD_REACTION				= permission(64);
	static const inline permission VIEW_AUDIT_LOG			= permission(128);
	static const inline permission VIEW_CHANNEL				= permission(1024);
	static const inline permission SEND_MESSAGES			= permission(2048);
	static const inline permission SEND_TTS_MESSAGES		= permission(4096);
	static const inline permission MANAGE_MESSAGES			= permission(8192);
	static const inline permission EMBED_LINKS				= permission(16384);
	static const inline permission ATTACH_FILES				= permission(32768);
	static const inline permission READ_MESSAGE_HISTORY		= permission(65536);
	static const inline permission MENTION_EVERYONE			= permission(131072);
	static const inline permission USE_EXTERNAL_EMOJIS		= permission(262144);
	static const inline permission CONNECT					= permission(1048576);
	static const inline permission SPEAK					= permission(2097152);
	static const inline permission MUTE_MEMBERS				= permission(4194304);
	static const inline permission DEAFEN_MEMBERS			= permission(8388608);
	static const inline permission MOVE_MEMBERS				= permission(16777216);
	static const inline permission USE_VAD					= permission(33554432);
	static const inline permission CHANGE_NICKNAME			= permission(67108864);
	static const inline permission MANAGE_NICKNAMES			= permission(134217728);
	static const inline permission MANAGE_ROLES				= permission(268435456);
	static const inline permission MANAGE_WEBHOOKS			= permission(536870912);
	static const inline permission MANAGE_EMOJIS			= permission(1073741824);
};

