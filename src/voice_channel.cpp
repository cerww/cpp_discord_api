#include "voice_channel.h"


int voice_channel::user_limit() const noexcept { return m_user_limit; }
int voice_channel::bitrate() const noexcept { return m_bitrate; }

void from_json(const nlohmann::json& json, voice_channel& out) {
	from_json(json, static_cast<guild_channel&>(out));
	out.m_user_limit = json["user_limit"].get<int>();
	out.m_bitrate = json["bitrate"].get<int>();
}
