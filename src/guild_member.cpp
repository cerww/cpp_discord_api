#include "guild_member.h"
#include "guild.h"


//Guild& guild_member::guild() noexcept { return *m_guild; }

const Guild& guild_member::guild() const noexcept { return *m_guild; }

Status guild_member::status() const noexcept {
	return m_status;
}

void from_json(const nlohmann::json& in, guild_member& out) {
	from_json(in, static_cast<partial_guild_member&>(out));
}

discord_obj_list<guild_role> guild_member::roles() const {
	return discord_obj_list<guild_role>(guild().roles(), role_ids());
}

void guild_member::set_presence(const partial_presence_update& presence_update) {
	m_status = presence_update.status();
	if (presence_update.game()) m_game = presence_update.game();
	else m_game = std::nullopt;
}

void to_json(nlohmann::json& out, const guild_member& in) {
	to_json(out, static_cast<const partial_guild_member&>(in));
}
