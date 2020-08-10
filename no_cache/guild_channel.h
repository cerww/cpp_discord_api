#pragma once
#include "partial_channel.h"
#include "../common/range-like-stuffs.h"
#include "partial_guild_channel.h"
#include <range/v3/all.hpp>
#include <span>

namespace cacheless {
struct channel_catagory;

struct Guild;

struct guild_channel :partial_guild_channel {

	friend struct internal_shard;
	friend void from_json(const nlohmann::json&, guild_channel& g);
};

void from_json(const nlohmann::json& json, guild_channel& g);
}
