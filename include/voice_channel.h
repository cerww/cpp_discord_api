#pragma once
#include "partial_channel.h"
#include "guild_channel.h"

struct voice_channel :guild_channel {
	int user_limit() const noexcept;
	int bitrate() const noexcept;

private:
	int m_user_limit = 0;
	int m_bitrate = 0;

	friend void from_json(const nlohmann::json& json, voice_channel& out);
	friend struct shard;
};

void from_json(const nlohmann::json& json, voice_channel& out);
