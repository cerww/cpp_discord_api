#include "partial_guild_member.h"

const std::vector<snowflake>& partial_guild_member::role_ids() const noexcept { return m_roles; }

timestamp partial_guild_member::joined_at() const noexcept { return m_joined_at; }

bool partial_guild_member::deaf() const noexcept { return m_deaf; }

bool partial_guild_member::mute() const noexcept { return m_mute; }

const std::string& partial_guild_member::nick() const noexcept { return m_nick.empty() ? username() : m_nick; }

bool partial_guild_member::has_role(const snowflake role_id) const noexcept {
	return std::find(m_roles.begin(), m_roles.end(), role_id) != m_roles.end();
}

bool partial_guild_member::has_role(const guild_role& role) const noexcept {
	return std::find(m_roles.begin(), m_roles.end(), role.id()) != m_roles.end();
}

bool partial_guild_member::has_role(guild_role&& role) const noexcept {
	return std::find(m_roles.begin(), m_roles.end(), role.id()) != m_roles.end();
}

