#pragma once
#include "Channel.h"
#include "guild_channel.h"

class channel_catagory:public guild_channel{
public:
	//wat
private:
	
	friend void from_json(const nlohmann::json& json, channel_catagory& c);
	friend class shard;
	friend class client;
	
};

inline void from_json(const nlohmann::json& json,channel_catagory& c) {
	from_json(json, static_cast<guild_channel&>(c));	
}
