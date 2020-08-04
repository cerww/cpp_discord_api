#include "presence_update.h"

namespace cacheless {


void from_json(const nlohmann::json& json, activity& thing) {
	thing.name = json["name"].get<std::string>();
	thing.type = json["type"].get<activity_type>();
}


void from_json(const nlohmann::json& json, partial_presence_update& thing) {
	thing.id = json["user"]["id"].get<snowflake>();
	thing.status = json["status"].get<Status>();
	const auto& t = json["game"];
	if (!t.is_null()) {
		thing.game.emplace(t.get<activity>());
	}
}
}
