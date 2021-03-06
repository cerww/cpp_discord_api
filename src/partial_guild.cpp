#include "partial_guild.h"
#include <fmt/core.h>

snowflake partial_guild::id() const noexcept { return m_id; }



std::string_view partial_guild::name() const noexcept { return m_name; }

std::string_view partial_guild::icon() const noexcept { return m_icon; }

snowflake partial_guild::owner_id() const noexcept { return m_owner_id; }

snowflake partial_guild::system_channel_id() const noexcept { return m_system_channel_id; }

std::string_view partial_guild::region() const noexcept { return m_region; }

int partial_guild::afk_timeout() const noexcept { return m_afk_timeout; }

snowflake partial_guild::afk_channel_id() const noexcept { return m_afk_channel_id; }

int partial_guild::verification_level() const noexcept { return m_verification_level; }

bool partial_guild::explicit_content_filter() const noexcept { return m_explicit_content_filter; }

void from_json(const nlohmann::json& json, partial_guild& guild) {
	guild.m_id = json["id"].get<snowflake>();
	//fmt::print("{}\n",guild.m_id.val);
	guild.m_name = json["name"].get<std::string>();
	guild.m_icon = json["icon"].is_null() ? "" : json["icon"].get<std::string>();
	guild.m_splash = json["splash"].is_null() ? "" : json["splash"].get<std::string>();

	guild.m_owner_id = json["owner_id"].get<snowflake>();
	guild.m_client_permissions = json.value("permissions", permission());
	guild.m_region = json["region"].get<std::string>();
	guild.m_afk_channel_id = json["afk_channel_id"].get<snowflake>();
	guild.m_afk_timeout = json["afk_timeout"].get<int>();
	guild.m_verification_level = json["verification_level"].get<int>();
	guild.m_default_message_notifications = json["default_message_notifications"].get<int>();
	guild.m_explicit_content_filter = json["explicit_content_filter"].get<int>();

	//guild.m_roles.reserve(json["roles"].size());
	guild.m_roles = json["roles"] | ranges::views::transform(&get_then_return_id<guild_role>) | ranges::to<ref_stable_map<snowflake,guild_role>>();
	
	//for (const auto& r : json["roles"]) 
		//insert_proj_as_key(guild.m_roles, r.get<guild_role>(), get_id);

	guild.m_emojis = json["emojis"].get<std::vector<emoji>>();
	guild.m_features = json["features"].get<std::vector<std::string>>();
	guild.m_mfa_level = json["mfa_level"].get<int>();
	const auto t = json.value("application_id", snowflake());
	if (t != snowflake())
		guild.m_application_id = t;

	guild.m_widget_enabled = json.value("widget_enabled", false);
	if(guild.m_widget_enabled) {
		auto t2 = json.value("widget_channel_id", snowflake());
		if (t2 != snowflake()) {
			guild.m_widget_channel_id = t;
		}
	}
	guild.m_system_channel_id = json["system_channel_id"].get<snowflake>();
}



