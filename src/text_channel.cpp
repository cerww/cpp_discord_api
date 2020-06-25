#include "text_channel.h"



std::string_view text_channel::topic() const noexcept { return m_topic; }

snowflake text_channel::last_message_id() const noexcept { return m_last_message_id; }

/*
guild_text_message& text_channel::p_add_msg(guild_text_message msg) {
	return m_msg_cache.add(std::move(msg));
}
*/

void from_json(const nlohmann::json& json, text_channel& channel) {
	from_json(json, static_cast<guild_channel&>(channel));
	const auto t = json["topic"];
	channel.m_topic = t.is_null() ? "" : t.get<std::string>();
	channel.m_last_message_id = json.value("last_message_id", snowflake{});
}
