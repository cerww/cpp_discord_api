#include "dm_channel.h"


snowflake dm_channel::last_message_id() const noexcept { return m_last_message_id; }

const std::vector<user>& dm_channel::recipients() const noexcept { return m_recipients; }
timestamp dm_channel::last_pin_timestamp() const noexcept { return m_last_pin_timestamp; }
const std::vector<dm_message>& dm_channel::msg_cache() const noexcept { return m_msg_cache.data(); }


dm_message& dm_channel::m_add_msg(dm_message msg) {
	return m_msg_cache.add(std::move(msg));
}

void to_json(nlohmann::json& json, const dm_channel& channel) {
	to_json(json, static_cast<const partial_channel&>(channel));
	json["recipients"] = channel.recipients();
	json["last_message_id"] = channel.last_message_id();
}

void from_json(const nlohmann::json& json, dm_channel& channel) {
	from_json(json, static_cast<partial_channel&>(channel));
	channel.m_recipients = json["recipients"].get<std::vector<user>>();
	channel.m_last_message_id = json["last_message_id"].get<snowflake>();
	//channel.m_last_pin_timestamp = json["last_pin_timestamp"];
}
