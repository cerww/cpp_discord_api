#pragma once
#include "channel.h"
#include "guild_channel.h"

class voice_channel :
	public guild_channel
{
public:
	int user_limit() const noexcept;
	int bitrate() const noexcept;

private:
	int m_user_limit = 0;	
	int m_bitrate = 0;

	friend void from_json(const nlohmann::json& json, voice_channel& out);
	friend class shard;
};

void from_json(const nlohmann::json& json, voice_channel& out);


