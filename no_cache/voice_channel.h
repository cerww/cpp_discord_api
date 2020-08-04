#pragma once
#include "partial_channel.h"
#include "guild_channel.h"

namespace cacheless {

struct voice_channel :guild_channel {
	int user_limit = 0;
	int bitrate = 0;

	friend void from_json(const nlohmann::json& json, voice_channel& out);
	friend struct internal_shard;
};

void from_json(const nlohmann::json& json, voice_channel& out);
}
