#pragma once
#include"snowflake.h"


namespace cacheless {
struct unavailable_guild {
	snowflake guild_id;
	bool unavailable = false;
};

inline void from_json(const nlohmann::json& json, unavailable_guild& g) {
	g.guild_id = json["guild_id"].get<snowflake>();
	g.unavailable = json["unavailable"].get<bool>();
}

}
