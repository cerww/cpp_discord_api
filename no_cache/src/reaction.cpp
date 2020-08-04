#include "reaction.h"

namespace cacheless {

void from_json(const nlohmann::json& json, reaction& r) {
	r.count = json["count"].get<int>();
	r.me = json["me"].get<bool>();
	r.emoji = json["emoji"].get<partial_emoji>();
}

}
