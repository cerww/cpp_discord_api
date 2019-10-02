#pragma once
#include "partial_channel.h"
#include "snowflake.h"
#include "timestamp.h"
#include "guild_member.h"
#include "partial_message.h"
#include "guild_channel.h"
#include "item_cache.h"

struct text_channel: guild_channel{
	auto msg_cache() const noexcept {
		return m_msg_cache.data() | ranges::views::all;
	};
	std::string_view topic() const noexcept;
	snowflake last_message_id() const noexcept;
private:
	std::string m_topic;
	snowflake m_last_message_id;
	
	timestamp m_last_pin_timestamp;

	dynamic_item_cache<guild_text_message> m_msg_cache = dynamic_item_cache<guild_text_message>(10ull,false);

	guild_text_message& p_add_msg(guild_text_message msg);

	friend void from_json(const nlohmann::json& json, text_channel& channel);
	friend struct shard;
};

void from_json(const nlohmann::json& json, text_channel& channel);

