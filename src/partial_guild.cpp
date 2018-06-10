#include "partial_guild.h"

snowflake partial_guild::id() const noexcept { return m_id; }

rename_later_4<snowflake, guild_role>& partial_guild::roles() noexcept { return m_roles; }

const rename_later_4<snowflake, guild_role>& partial_guild::roles() const noexcept { return m_roles; }

const std::string& partial_guild::name() const noexcept { return m_name; }

const std::string& partial_guild::icon() const noexcept { return m_icon; }

snowflake partial_guild::owner_id() const noexcept { return m_owner_id; }

std::vector<emoji>& partial_guild::emojis() noexcept { return m_emojis; }

const std::vector<emoji>& partial_guild::emojis() const noexcept { return m_emojis; }

snowflake partial_guild::general_channel_id() const noexcept { return m_system_channel_id; }
const std::string& partial_guild::region() const noexcept { return m_region; }
int partial_guild::afk_timeout() const noexcept { return m_afk_timeout; }
snowflake partial_guild::afk_channel_id() const noexcept { return m_afk_channel_id; }
bool partial_guild::embed_enabled() const noexcept { return m_embed_enabled; }
snowflake partial_guild::embed_channel_id() const noexcept { return m_embed_channel_id; }
int partial_guild::verification_level() const noexcept { return m_verification_level; }
bool partial_guild::explicit_content_filter() const noexcept { return m_explicit_content_filter; }
const std::vector<std::string>& partial_guild::features() const noexcept { return m_features; }

void from_json(const nlohmann::json& json, partial_guild& guild) {
	guild.m_id = json["id"].get<snowflake>();
	guild.m_name = json["name"].get<std::string>();
	guild.m_icon = json["icon"].is_null() ? "" : json["icon"].get<std::string>();
	guild.m_splash = json["splash"].is_null() ? "" : json["splash"].get<std::string>();

	guild.m_owner_id = json["owner_id"].get<snowflake>();
	guild.m_client_permissions = json.value("permissions", permission());
	guild.m_region = json["region"].get<std::string>();
	guild.m_afk_channel_id = json["afk_channel_id"].get<snowflake>();
	guild.m_afk_timeout = json["afk_timeout"].get<int>();
	guild.m_embed_enabled = json.value("embed_enabled", false);
	guild.m_embed_channel_id = json.value("embed_channel_id", snowflake());
	guild.m_verification_level = json["verification_level"].get<int>();
	guild.m_default_message_notifications = json["default_message_notifications"].get<int>();
	guild.m_explicit_content_filter = json["explicit_content_filter"].get<int>();

	guild.m_roles.reserve(json["roles"].size());
	for (auto& r : json["roles"]) insert_proj_as_key(guild.m_roles, r.get<guild_role>(), get_id);

	guild.m_emojis = json["emojis"].get<std::vector<emoji>>();
	guild.m_features = json["features"].get<std::vector<std::string>>();
	guild.m_mfa_level = json["mfa_level"].get<int>();
	//guild.m_application_id = 
	//widget
	//widget_channel
	guild.m_system_channel_id = json["system_channel_id"].get<snowflake>();
}



