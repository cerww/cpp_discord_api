#pragma once
#include "Channel.h"
#include "text_channel.h"
#include "Role.h"
#include "guildMember.h"
#include <nlohmann/json.hpp>
#include "emoji.h"
#include "snowflake.h"
#include "timestamp.h"
#include <optional>
#include "voice_channel.h"
#include "partial_guild.h"
#include "voice_state.h"

class shard;

class Guild:public partial_guild
{
public:
	timestamp joined_at() const noexcept;
	bool large() const noexcept;
	bool unavailable() const noexcept;
	int member_count() const noexcept;
	text_channel& general_channel() const noexcept;
	const std::vector<guild_member>& members() const noexcept;
	std::vector<guild_member>& members() noexcept;
	thing<text_channel> text_channels()const;
	thing<voice_channel> voice_channels()const;
	thing<channel_catagory> channel_catagories()const;
	voice_channel* afk_channel()const noexcept;
	const std::vector<voice_state>& voice_states() const noexcept;
private:
	timestamp m_joined_at = {};
	bool m_large = false;
	bool m_unavailable = false;
	int m_member_count = 0;
	std::vector<guild_member> m_members;
	std::vector<snowflake> m_text_channels;	
	std::vector<snowflake> m_voice_channels;
	std::vector<snowflake> m_channel_catagories;
	std::vector<voice_state> m_voice_states;
	
	shard* m_shard = nullptr;
	
	friend void from_json(const nlohmann::json& json, Guild& guild);
	friend class shard;
};

void from_json(const nlohmann::json& json, Guild& guild);





