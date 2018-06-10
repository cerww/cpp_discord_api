#pragma once
#include "partial_channel.h"
#include "snowflake.h"
#include "timestamp.h"
#include "guildMember.h"
#include "partial_message.h"
#include "guild_channel.h"
#include "item_cache.h"

class text_channel:public guild_channel{
public:
	const std::vector<guild_text_message>& msg_cache() const noexcept;
	std::vector<guild_text_message>& msg_cache() noexcept;
	const std::string& topic() const noexcept;
	snowflake last_message_id() const noexcept;
private:
	std::string m_topic;
	snowflake m_last_message_id;
	
	timestamp m_last_pin_timestamp;

	dynamic_item_cache<guild_text_message> m_msg_cache = dynamic_item_cache<guild_text_message>(10ull,false);

	void m_add_msg(guild_text_message msg);

	friend void from_json(const nlohmann::json& json, text_channel& channel);
	friend class shard;
};

void from_json(const nlohmann::json& json, text_channel& channel);

