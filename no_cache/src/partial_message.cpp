#include "partial_message.h"
#include "partial_channel.h"
#include "text_channel.h"
#include "guild.h"

namespace cacheless {


void from_json(const nlohmann::json& json, partial_message& msg) {
	msg.author_id = json["author"]["id"].get<snowflake>();
	msg.id = json["id"].get<snowflake>();
	msg.channel_id = json["channel_id"].get<snowflake>();
	//
	msg.content = json["content"].get<std::string>();
	msg.mention_everyone = json["mention_everyone"].get<bool>();
	msg.timestamp = json["timestamp"].get<timestamp>();
	//msg.m_edited_timestamp = json["edited_timestamp"].get<std::optional<timestamp>>();
	msg.tts = json["tts"].get<bool>();
	msg.mention_everyone = json["mention_everyone"].get<bool>();
	msg.attachments = json["attachments"].get<std::vector<attachment>>();
	msg.reactions = json.value("reactions", std::vector<reaction>());

}

void from_json(const nlohmann::json& json, msg_update& msg) {
	const auto it = json.find("author");
	if (it != json.end())
		msg.author_id = (*it)["id"].get<snowflake>();

	msg.id = json["id"].get<snowflake>();
	msg.channel_id = json["channel_id"].get<snowflake>();

	msg.content = json.value("content", std::string());
	msg.mention_everyone = json.value("mention_everyone", false);
	//msg.m_edited_timestamp = json["edited_timestamp"].get<std::optional<timestamp>>();

	msg.tts = json.value("tts", false);
	msg.mention_everyone = json.value("mention_everyone", std::optional<bool>());
	msg.attachments = json.value("attachments", std::vector<attachment>());
	msg.reactions = json.value("reactions", std::vector<reaction>());
}

}
