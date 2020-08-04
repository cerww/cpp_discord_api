#include "dm_channel.h"

namespace cacheless {




void to_json(nlohmann::json& json, const dm_channel& channel) {
	to_json(json, static_cast<const partial_channel&>(channel));
	//json["recipients"] = channel.recipients();
	json["last_message_id"] = channel.last_message_id;
}

void from_json(const nlohmann::json& json, dm_channel& channel) {
	from_json(json, static_cast<partial_channel&>(channel));

	channel.recipients = json["recipients"] | ranges::views::transform(&get_then_return_id<user>) | ranges::to<ref_stable_map<snowflake, user>>();


	channel.last_message_id = json["last_message_id"].get<snowflake>();
}
}
