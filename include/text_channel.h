#pragma once
#include "partial_channel.h"
#include "snowflake.h"
#include "timestamp.h"
#include "guild_member.h"
#include "partial_message.h"
#include "guild_channel.h"
#include "../common/item_cache.h"

enum class text_channel_type {
	normal,
	store,
	news
};

struct text_channel :guild_channel {
	
	/*auto msg_cache() const noexcept {
		return m_msg_cache.data() | ranges::views::all;
	};
	*/

	std::string_view topic() const noexcept { return m_topic; };
	//snowflake last_message_id() const noexcept { return m_last_message_id; };

	bool is_news_channel()const noexcept {
		return m_channel_type == text_channel_type::news;
	}

	bool is_store_channel()const noexcept {
		return m_channel_type == text_channel_type::store;
	}

	bool is_normal_channel()const noexcept {
		return m_channel_type == text_channel_type::normal;
	}
private:
	std::string m_topic;
	snowflake m_last_message_id;

	timestamp m_last_pin_timestamp;
	
	text_channel_type m_channel_type = text_channel_type::normal;

	//dynamic_item_cache<guild_text_message> m_msg_cache = dynamic_item_cache<guild_text_message>(10ull, false);

	//guild_text_message& p_add_msg(guild_text_message msg);

	friend void from_json(const nlohmann::json& json, text_channel& channel);
	friend struct internal_shard;
};

void from_json(const nlohmann::json& json, text_channel& channel);
