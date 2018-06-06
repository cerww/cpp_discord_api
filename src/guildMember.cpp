#include "guildMember.h"
#include "guild.h"


Guild& guild_member::guild() noexcept { return *m_guild; }

const Guild& guild_member::guild() const noexcept { return *m_guild; }

const std::string& guild_member::nick() const noexcept { return m_nick.empty() ? username() : m_nick; }

const std::vector<snowflake>& guild_member::role_ids() const noexcept { return m_roles; }

timestamp guild_member::joined_at() const noexcept { return m_joined_at; }

bool guild_member::deaf() const noexcept { return m_deaf; }

bool guild_member::mute() const noexcept { return m_mute; }

Status guild_member::status() const noexcept {
	return m_status;
}

void guild_member::set_presence(const partial_presence_update& presence_update) {
	m_status = presence_update.status();
	if (presence_update.game()) m_game = presence_update.game();
	else m_game = std::nullopt;
}

void from_json(const nlohmann::json& in, guild_member& out) {
	from_json(in["user"], static_cast<User&>(out));
	const auto it = in.find("nick");
	if (it != in.end()) out.m_nick = in["nick"].is_null() ? "" : in["nick"].get<std::string>();
	out.m_roles = in["roles"].get<std::vector<snowflake>>();
	out.m_deaf = in["deaf"].get<bool>();
	out.m_mute = in["mute"].get<bool>();
}

discord_obj_list<Role> guild_member::roles() const {
	return discord_obj_list<Role>(m_guild->roles(), m_roles);
}

bool guild_member::has_role(const snowflake role_id) const noexcept {
	return std::find(m_roles.begin(), m_roles.end(), role_id) != m_roles.end();
}

bool guild_member::has_role(const Role& role) const noexcept {
	return std::find(m_roles.begin(), m_roles.end(), role.id()) != m_roles.end();
}

bool guild_member::has_role(Role&& role) const noexcept {
	return std::find(m_roles.begin(), m_roles.end(), role.id()) != m_roles.end();
}

void to_json(nlohmann::json& out, const guild_member& in) {
	to_json(out, static_cast<const User&>(in));
	out["nick"] = in.nick();
	//out["roles"] = in.roles();
	out["deaf"] = in.deaf();
	out["mute"] = in.mute();
}
