#include "partial_guild_member.h"


timestamp partial_guild_member::joined_at() const noexcept {
	return m_joined_at;
}

bool partial_guild_member::deaf() const noexcept {
	return m_deaf;
}

bool partial_guild_member::mute() const noexcept {
	return m_mute;
}

std::string_view partial_guild_member::nick() const noexcept {
	return m_nick.empty() ? username() : std::string_view(m_nick.data(), m_nick.size());
}

bool partial_guild_member::has_role(const snowflake role_id) const noexcept {
	return std::ranges::find(m_roles, role_id) != m_roles.end();
}

bool partial_guild_member::has_role(const guild_role& role) const noexcept {
	return has_role(role.id());
}



