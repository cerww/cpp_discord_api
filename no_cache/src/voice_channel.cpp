#include "voice_channel.h"

namespace cacheless {


void from_json(const nlohmann::json& json, voice_channel& out) {
	from_json(json, static_cast<guild_channel&>(out));
	out.user_limit = json["user_limit"].get<int>();
	out.bitrate = json["bitrate"].get<int>();
}
}
