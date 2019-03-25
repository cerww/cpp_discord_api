#pragma once
#include "partial_channel.h"
#include "guild_channel.h"

struct channel_catagory:guild_channel{
	//wat
private:
	
	friend void from_json(const nlohmann::json& json, channel_catagory& c);
	friend struct shard;
	friend struct client;
};

inline void from_json(const nlohmann::json& json,channel_catagory& c) {
	from_json(json, static_cast<guild_channel&>(c));	
}
