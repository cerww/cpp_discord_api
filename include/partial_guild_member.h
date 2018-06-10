#pragma once
#include "user.h"
#include "timestamp.h"
#include "guild_role.h"

struct partial_guild_member:user{
	const std::string& nick() const noexcept;
	const std::vector<snowflake>& role_ids() const noexcept;
	timestamp joined_at() const noexcept;
	bool deaf() const noexcept;
	bool mute() const noexcept;

	bool has_role(const snowflake role_id) const noexcept;
	bool has_role(const guild_role& role) const noexcept;
	bool has_role(guild_role&& role) const noexcept;

private:
	std::string m_nick;
	std::vector<snowflake> m_roles;
	timestamp m_joined_at;
	bool m_deaf = false;
	bool m_mute = false;

	friend void from_json(const nlohmann::json&, partial_guild_member&);
	friend class shard;
};


inline void from_json(const nlohmann::json& in, partial_guild_member& out) {
	const auto it = in.find("nick");
	if (it != in.end()) out.m_nick = in["nick"].is_null() ? "" : in["nick"].get<std::string>();
	out.m_roles = in["roles"].get<std::vector<snowflake>>();
	//out.m_joined_at = in["joined_at"].get<timestamp>();
	out.m_deaf = in["deaf"].get<bool>();
	out.m_mute = in["mute"].get<bool>();
}
