#pragma once
#include "snowflake.h"
#include "partial_channel.h"

#include <span>

namespace cacheless {
struct partial_guild_channel :partial_channel {
	snowflake guild_id;
	bool nsfw = false;
	int position = 0;
	std::vector<permission_overwrite> permission_overwrites;

	snowflake parent_id;
	
	friend void from_json(const nlohmann::json& json, partial_guild_channel& g);
	friend struct internal_shard;
};


inline void from_json(const nlohmann::json& json, partial_guild_channel& g) {
	from_json(json, static_cast<partial_channel&>(g));
	g.guild_id = json.value("guild_id", snowflake());
	g.nsfw = json.value("nsfw", false);
	g.position = json["position"].get<int>();
	g.permission_overwrites = json["permission_overwrites"].get<std::vector<permission_overwrite>>();
	g.parent_id = json.value("parent_id", snowflake());
}
};
namespace potato {
	struct asdasd {
	};
}