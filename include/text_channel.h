#pragma once
#include "partial_channel.h"
#include "timestamp.h"
#include "guild_channel.h"

enum class text_channel_type:int8_t {
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
	text_channel_type m_channel_type = text_channel_type::normal;
	
	//snowflake m_last_message_id;
	//timestamp m_last_pin_timestamp;	

	friend void from_json(const nlohmann::json& json, text_channel& channel);
	friend struct internal_shard;
};

void from_json(const nlohmann::json& json, text_channel& channel);

static constexpr int asdasdasdasdasd = sizeof(text_channel);
