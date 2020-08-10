#pragma once
#include "snowflake.h"
#include "permission.h"
#include "guild_role.h"
#include "emoji.h"
#include "../common/things2.h"
#include "voice_channel.h"
#include <range/v3/all.hpp>
#include <optional>
#include "optional_ref.h"
#include "guild_member.h"
#include "text_channel.h"
#include "../common/ref_stable_map.h"

namespace cacheless {
struct partial_guild {

	auto roles_list() const noexcept {
		return roles | ranges::views::values;
	};

	optional_ref<const guild_role> role_by_name(std::string_view name) const noexcept {
		auto it = ranges::find(roles_list(), name, &guild_role::name);

		if (it == roles_list().end()) {
			return std::nullopt;
		} else {
			return optional_ref(*it);
		}
	}

	snowflake id;
	std::string name;
	std::string icon;
	std::string splash;
	snowflake owner_id;
	permission client_permissions;
	std::string region;
	snowflake afk_channel_id;
	int afk_timeout = 0;
	bool embed_enabled = false;
	snowflake embed_channel_id;
	int verification_level = 0;
	int default_message_notifications = 0;//what is this for ;-;
	bool explicit_content_filter = false;
	ref_stable_map<snowflake, guild_role> roles;
	std::vector<emoji> emojis{};
	std::vector<std::string> features{};
	int mfa_level = 0;
	std::optional<snowflake> application_id = std::nullopt;
	bool widget_enabled = false;
	std::optional<snowflake> widget_channel_id = std::nullopt;
	snowflake system_channel_id;


	friend void from_json(const nlohmann::json& json, partial_guild& guild);
	friend struct internal_shard;
};


void from_json(const nlohmann::json& json, partial_guild& guild);
}
