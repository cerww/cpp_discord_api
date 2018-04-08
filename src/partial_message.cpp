#include "partial_message.h"
#include "channel.h"
#include "text_channel.h"
#include "guild.h"


const std::string& partial_message::content() const noexcept { return m_content; }
snowflake partial_message::id() const noexcept { return m_id; }
snowflake partial_message::author_id() const noexcept { return m_author_id; }
snowflake partial_message::channel_id() const noexcept { return m_channel_id; }
bool partial_message::tts() const noexcept { return m_tts; }
bool partial_message::mention_everyone() const noexcept { return m_mention_everyone; }
const std::vector<reaction>& partial_message::reactions() const noexcept { return m_reactions; }
const std::vector<attachment>& partial_message::attachments() const noexcept { return m_attachments; }

text_channel& guild_text_message::channel() noexcept { return *m_channel; }
const text_channel& guild_text_message::channel() const noexcept { return *m_channel; }
guild_member& guild_text_message::author() noexcept { return *m_author; }
const guild_member& guild_text_message::author() const noexcept { return *m_author; }
const std::vector<snowflake>& guild_text_message::mention_roles() const noexcept { return m_mention_roles; }
const std::vector<guild_member*>& guild_text_message::mentions() const noexcept { return m_mentions; }

User& dm_message::author() noexcept { return *m_author; }
const User& dm_message::author() const noexcept { return *m_author; }
const std::vector<User*>& dm_message::mentions() const noexcept { return m_mentions; }
dm_channel& dm_message::channel() noexcept { return *m_channel; }
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
const std::string& msg_update::content() const noexcept { return m_content; }
User& dm_msg_update::author() { return *m_author; }
const User& dm_msg_update::author() const { return *m_author; }

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

Guild& guild_text_message::guild() noexcept { return channel().guild(); }
const Guild& guild_text_message::guild() const noexcept {return channel().guild(); }
