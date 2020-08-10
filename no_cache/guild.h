#pragma once
#include "snowflake.h"
#include "partial_channel.h"
#include "text_channel.h"
#include "guild_member.h"
#include <nlohmann/json.hpp>
#include "timestamp.h"
#include "voice_channel.h"
#include "partial_guild.h"
#include "voice_state.h"
#include "../common/higher_order_functions.h"
#include "channel_catagory.h"
#include "presence_update.h"
#include "../common/ref_stable_map.h"

namespace cacheless {

struct internal_shard;

struct Guild :partial_guild {

	Guild() = default;
	Guild(const Guild&) = delete;
	Guild(Guild&&) = default;

	Guild& operator=(const Guild&) = delete;
	Guild& operator=(Guild&&) = default;


	timestamp joined_at = {};
	bool large = false;
	bool unavailable = false;
	int member_count = 0;

	ska::bytell_hash_map<snowflake, guild_member> members{};
	ska::bytell_hash_map<snowflake, std::optional<activity>> activities;//no optional<activity&> ;-;
	
	ska::bytell_hash_map<snowflake, text_channel> text_channels;
	ska::bytell_hash_map<snowflake, voice_channel> voice_channels;
	ska::bytell_hash_map<snowflake, channel_catagory> channel_catagories;


	std::vector<snowflake> text_channel_ids{};
	std::vector<snowflake> voice_channel_ids{};
	std::vector<snowflake> channel_catagory_ids{};
	std::vector<voice_state> voice_states{};



	friend void from_json(const nlohmann::json& json, Guild& guild);
	friend struct internal_shard;
};

}
