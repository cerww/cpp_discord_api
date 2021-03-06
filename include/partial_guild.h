#pragma once
#include "snowflake.h"
#include "permission.h"
#include "guild_role.h"
#include "emoji.h"
#include "../common/things2.h"
#include "voice_channel.h"
#include <range/v3/all.hpp>
#include <optional>
#include "../common/optional_ref.h"

struct partial_guild {
	snowflake id() const noexcept;
	discord_obj_map<guild_role> roles() const noexcept { return m_roles; };
	
	std::string_view name() const noexcept;
	std::string_view icon() const noexcept;
	snowflake owner_id() const noexcept;

	std::span<const emoji> emojis() const noexcept {
		return m_emojis;
	}

	snowflake system_channel_id() const noexcept;
	std::string_view region() const noexcept;
	int afk_timeout() const noexcept;
	snowflake afk_channel_id() const noexcept;
	int verification_level() const noexcept;
	bool explicit_content_filter() const noexcept;

	std::span<const std::string> features() const noexcept {
		return m_features;
	}	

	auto roles_list() const noexcept {
		return m_roles | ranges::views::values;
	};

	std::optional<guild_role> role_by_name(std::string_view name)const noexcept {
		auto it = ranges::find(roles_list(), name, &guild_role::name);
		
		if(it == roles_list().end()) {
			return std::nullopt;
		}else {
			return *it;
		}		
	}
	
private:
	std::string m_icon;
	std::string m_splash;
	snowflake m_id;
	std::string m_name;
	snowflake m_owner_id;
	permission m_client_permissions;
	std::string m_region;
	snowflake m_afk_channel_id;
	int m_afk_timeout = 0;
	int m_verification_level = 0;
	int m_default_message_notifications = 0;//what is this for ;-;
	bool m_explicit_content_filter = false;
	ref_stable_map<snowflake, guild_role> m_roles{};
	std::vector<emoji> m_emojis{};
	std::vector<std::string> m_features{};
	int m_mfa_level = 0;
	std::optional<snowflake> m_application_id = std::nullopt;
	bool m_widget_enabled = false;
	std::optional<snowflake> m_widget_channel_id = std::nullopt;
	snowflake m_system_channel_id;

	
	friend void from_json(const nlohmann::json& json, partial_guild& guild);
	friend struct internal_shard;
};


void from_json(const nlohmann::json& json, partial_guild& guild);
