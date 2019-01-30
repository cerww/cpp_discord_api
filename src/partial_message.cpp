#include "partial_message.h"
#include "partial_channel.h"
#include "text_channel.h"
#include "guild.h"


std::string_view partial_message::content() const noexcept { return m_content; }
snowflake partial_message::id() const noexcept { return m_id; }
snowflake partial_message::author_id() const noexcept { return m_author_id; }
snowflake partial_message::channel_id() const noexcept { return m_channel_id; }
bool partial_message::tts() const noexcept { return m_tts; }
bool partial_message::mention_everyone() const noexcept { return m_mention_everyone; }

const text_channel& guild_text_message::channel() const noexcept { return *m_channel; }
const guild_member& guild_text_message::author() const noexcept { return *m_author; }

const user& dm_message::author() const noexcept { return *m_author; }
const dm_channel& dm_message::channel() const noexcept { return *m_channel; }

void from_json(const nlohmann::json& json, partial_message& msg) {
	msg.m_author_id = json["author"]["id"].get<snowflake>();
	msg.m_id = json["id"].get<snowflake>();
	msg.m_channel_id = json["channel_id"].get<snowflake>();
	//
	msg.m_content = json["content"].get<std::string>();
	msg.m_mention_everyone = json["mention_everyone"].get<bool>();
	msg.m_timestamp = json["timestamp"].get<timestamp>();
	//msg.m_edited_timestamp = json["edited_timestamp"].get<std::optional<timestamp>>();
	msg.m_tts = json["tts"].get<bool>();
	msg.m_mention_everyone = json["mention_everyone"].get<bool>();
	msg.m_attachments = json["attachments"].get<std::vector<attachment>>();
	msg.m_reactions = json.value("reactions", std::vector<reaction>());

}

snowflake msg_update::id() const noexcept { return m_id; }
snowflake msg_update::channel_id() const noexcept { return m_channel_id; }
std::string_view msg_update::content() const noexcept { return m_content; }
const guild_member& guild_msg_update::author() const noexcept {return *m_author;}

const text_channel& guild_msg_update::channel() const noexcept { return *m_channel; }
//user& dm_msg_update::author() { return *m_author; }
const user& dm_msg_update::author() const { return *m_author; }

const dm_channel& dm_msg_update::channel() const noexcept { return *m_channel; }

void from_json(const nlohmann::json& json, msg_update& msg) {
	const auto it = json.find("author");
	if (it != json.end()) msg.m_author_id = (*it)["id"].get<snowflake>();

	msg.m_id = json["id"].get<snowflake>();
	msg.m_channel_id = json["channel_id"].get<snowflake>();
	msg.m_content = json.value("content", std::string());
	msg.m_mention_everyone = json.value("mention_everyone", false);
	//msg.m_edited_timestamp = json["edited_timestamp"].get<std::optional<timestamp>>();
	msg.m_tts = json.value("tts", false);
	msg.m_mention_everyone = json.value("mention_everyone", false);
	msg.m_attachments = json.value("attachments", std::vector<attachment>());
	msg.m_reactions = json.value("reactions", std::vector<reaction>());
}

const Guild& guild_text_message::guild() const noexcept {return channel().guild(); }
