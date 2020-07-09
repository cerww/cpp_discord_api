#pragma once
#include "text_channel.h"

struct store_channel:guild_channel {
	//????
private:
	friend void from_json(const nlohmann::json& json,store_channel& channel) {
		from_json(json, static_cast<guild_channel&>(channel));
	}
};


