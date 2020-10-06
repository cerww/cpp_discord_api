#include "guild_member.h"
#include "guild.h"


//Guild& guild_member::guild() noexcept { return *m_guild; }

const Guild& guild_member::guild() const noexcept { return *m_guild; }



void from_json(const nlohmann::json& in, guild_member& out) {
	from_json(in, static_cast<partial_guild_member&>(out));
}

discord_obj_map<guild_role> guild_member::id_to_role_map() const noexcept {
	return m_guild->roles();
}

snowflake guild_member::guild_id() const noexcept {
	return guild().id();
};


void to_json(nlohmann::json& out, const guild_member& in) {
	to_json(out, static_cast<const partial_guild_member&>(in));
}
