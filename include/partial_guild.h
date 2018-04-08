#pragma once
#include "snowflake.h"
#include "permission.h"
#include "Role.h"
#include "emoji.h"
#include "things2.h"
#include "voice_channel.h"

class partial_guild{
public:
	snowflake id() const noexcept;
	std::unordered_map<snowflake, Role>& roles() noexcept;
	const std::unordered_map<snowflake, Role>& roles() const noexcept;
	const std::string& name() const noexcept;
	const std::string& icon() const noexcept;
	snowflake owner_id() const noexcept;
	std::vector<emoji>& emojis() noexcept;
	const std::vector<emoji>& emojis() const noexcept;
	snowflake general_channel_id()const noexcept;
	const std::string& region() const noexcept;
	int afk_timeout() const noexcept;
	snowflake afk_channel_id() const noexcept;
	bool embed_enabled() const noexcept;
	snowflake embed_channel_id() const noexcept;
	int verification_level() const noexcept;
	bool explicit_content_filter() const noexcept;
	const std::vector<std::string>& features() const noexcept;
private:
	snowflake m_id;
	std::string m_name;
	std::string m_icon;
	std::string m_splash;
	snowflake m_owner_id;
	permission m_client_permissions;
	std::string m_region;
	snowflake m_afk_channel_id;
	int m_afk_timeout = 0;
	bool m_embed_enabled = false;
	snowflake m_embed_channel_id;
	int m_verification_level = 0;
	int m_default_message_notifications = 0;//what is this for ;-;
	bool m_explicit_content_filter = false;
	std::unordered_map<snowflake, Role> m_roles;
	std::vector<emoji> m_emojis;
	std::vector<std::string> m_features;
	int m_mfa_level = 0;
	//application_id
	//widget_enabled
	//widget_channel_id
	snowflake m_system_channel_id;
	friend void from_json(const nlohmann::json& json, partial_guild& guild);
	friend class shard;
};


void from_json(const nlohmann::json& json, partial_guild& guild);
