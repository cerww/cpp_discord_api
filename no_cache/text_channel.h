#pragma once
#include "partial_channel.h"
#include "snowflake.h"
#include "timestamp.h"
#include "guild_member.h"
#include "partial_message.h"
#include "guild_channel.h"
#include "item_cache.h"
#include <string>


namespace cacheless {
enum class text_channel_type {
	normal,
	store,
	news
};

struct text_channel :guild_channel {

	std::string topic;
	snowflake last_message_id;

	timestamp last_pin_timestamp;

	text_channel_type channel_type = text_channel_type::normal;

	friend void from_json(const nlohmann::json& json, text_channel& channel);

	friend struct internal_shard;
};

}
