#include "text_channel.h"


const std::vector<guild_text_message>& text_channel::msg_cache() const noexcept { return m_msg_cache.data(); }
std::vector<guild_text_message>& text_channel::msg_cache() noexcept { return m_msg_cache.data(); }
const std::string& text_channel::topic() const noexcept { return m_topic; }
snowflake text_channel::last_message_id() const noexcept { return m_last_message_id; }

void text_channel::m_add_msg(guild_text_message msg) {
	m_msg_cache.add(std::move(msg));
}

void from_json(const nlohmann::json& json, text_channel& channel) {
	from_json(json, static_cast<guild_channel&>(channel));
	const auto t = json["topic"];
	channel.m_topic = t.is_null() ? "" : t.get<std::string>();
	channel.m_last_message_id = json.value("last_message_id", snowflake{});
}
