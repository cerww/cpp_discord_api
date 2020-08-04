#include "partial_guild.h"
#include <fmt/core.h>

namespace cacheless {

void from_json(const nlohmann::json& json, partial_guild& guild) {
	guild.id = json["id"].get<snowflake>();
	//fmt::print("{}\n",guild.m_id.val);
	guild.name = json["name"].get<std::string>();
	guild.icon = json["icon"].is_null() ? "" : json["icon"].get<std::string>();
	guild.splash = json["splash"].is_null() ? "" : json["splash"].get<std::string>();

	guild.owner_id = json["owner_id"].get<snowflake>();
	guild.client_permissions = json.value("permissions", permission());
	guild.region = json["region"].get<std::string>();
	guild.afk_channel_id = json["afk_channel_id"].get<snowflake>();
	guild.afk_timeout = json["afk_timeout"].get<int>();
	guild.embed_enabled = json.value("embed_enabled", false);
	guild.embed_channel_id = json.value("embed_channel_id", snowflake());
	guild.verification_level = json["verification_level"].get<int>();
	guild.default_message_notifications = json["default_message_notifications"].get<int>();
	guild.explicit_content_filter = json["explicit_content_filter"].get<int>();

	guild.roles.reserve(json["roles"].size());
	guild.roles = json["roles"] | ranges::views::transform(&get_then_return_id<guild_role>) | ranges::to<ref_stable_map<snowflake, guild_role>>();

	//for (const auto& r : json["roles"]) 
	//insert_proj_as_key(guild.m_roles, r.get<guild_role>(), get_id);

	guild.emojis = json["emojis"].get<std::vector<emoji>>();
	guild.features = json["features"].get<std::vector<std::string>>();
	guild.mfa_level = json["mfa_level"].get<int>();
	
	const auto t = json.value("application_id", snowflake());
	
	if (t != snowflake()) {
		guild.application_id = t;
	}
	
	guild.widget_enabled = json.value("widget_enabled", false);
	if (guild.widget_enabled) {
		auto t2 = json.value("widget_channel_id", snowflake());
		if (t2 != snowflake()) {
			guild.widget_channel_id = t;
		}
	}
	guild.system_channel_id = json["system_channel_id"].get<snowflake>();
}


};
