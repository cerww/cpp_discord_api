#include "partial_message.h"
#include "partial_channel.h"
#include "guild.h"


snowflake partial_message::id() const noexcept { return m_id; }

snowflake partial_message::author_id() const noexcept { return m_author_id; }

bool partial_message::tts() const noexcept { return m_tts; }

bool partial_message::mention_everyone() const noexcept { return m_mention_everyone; }

void from_json(const nlohmann::json& json, partial_message& msg) {
	msg.m_author_id = json["author"]["id"].get<snowflake>();
	msg.m_id = json["id"].get<snowflake>();
	msg.m_channel_id = json["channel_id"].get<snowflake>();
	
	msg.m_content = json["content"].get<std::string>();
	//msg.m_content.reserve(sizeof(std::string));//disable SBO
	
	msg.m_mention_everyone = json["mention_everyone"].get<bool>();
	msg.m_timestamp = json["timestamp"].get<timestamp>();
	
	//msg.m_edited_timestamp = json["edited_timestamp"].get<std::optional<timestamp>>();
	msg.m_tts = json["tts"].get<bool>();
	msg.m_mention_everyone = json["mention_everyone"].get<bool>();
	msg.m_attachments = json["attachments"].get<lol_wat_vector<attachment>>();
	msg.m_reactions = json.value("reactions", lol_wat_vector<reaction>());
	msg.m_embeds = json["embeds"].get<lol_wat_vector<embed>>();
	msg.m_type = (message_type)json["type"].get<int>();
	
	// msg.m_attachments = json["attachments"].get<std::vector<attachment>>();
	// msg.m_reactions = json.value("reactions", std::vector<reaction>());
	// msg.m_embeds = json["embeds"].get<std::vector<embed>>();
}

snowflake msg_update::id() const noexcept {
	return m_id;
}

snowflake msg_update::channel_id() const noexcept { return m_channel_id; }

std::optional<std::string_view> msg_update::content() const noexcept {
	if (m_content.empty()) {
		return std::nullopt;
	} else {
		return std::string_view(m_content);
	}
}

void from_json(const nlohmann::json& json, msg_update& msg) {
	const auto it = json.find("author");
	if (it != json.end())
		msg.m_author_id = (*it)["id"].get<snowflake>();

	msg.m_id = json["id"].get<snowflake>();
	msg.m_channel_id = json["channel_id"].get<snowflake>();
	
	msg.m_content = json.value("content", std::string());
	msg.m_mention_everyone = json.value("mention_everyone", false);
	//msg.m_edited_timestamp = json["edited_timestamp"].get<std::optional<timestamp>>();
	
	msg.m_tts = json.value("tts", false);
	msg.m_mention_everyone = json.value("mention_everyone", std::optional<bool>());
	msg.m_attachments = json.value("attachments", std::vector<attachment>());
	msg.m_reactions = json.value("reactions", std::vector<reaction>());
}

