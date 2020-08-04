#pragma once
#include <string>
#include <vector>
#include "guild_member.h"
#include "attachment.h"
#include "reaction.h"
#include <optional>
#include <range/v3/all.hpp>
#include <span>
#include "embed.h"

namespace cacheless {

struct dm_channel;
struct text_channel;
struct Guild;

struct partial_message {
	snowflake author_id;
	snowflake id;
	snowflake channel_id;
	std::vector<embed> embeds;
	std::vector<attachment> attachments;
	std::vector<reaction> reactions;
	std::string content;
	std::optional<timestamp> edited_timestamp;
	timestamp timestamp;
	bool tts = false;
	bool mention_everyone = false;


	friend struct client;
	friend struct internal_shard;
	friend void from_json(const nlohmann::json& json, partial_message& msg);
	friend struct msg_update_access;
};


struct guild_text_message :partial_message {

	guild_member author;
	snowflake guild_id;
	std::vector<snowflake> mention_roles_ids;
	std::vector<snowflake> mentions_ids;
	std::vector<guild_member> mentions;
	
	friend struct internal_shard;
	friend struct msg_update_access;
};

static constexpr int asueohdasd = sizeof(std::vector<snowflake>);

static constexpr int rawradsjksdfhksldjfa = sizeof(guild_member);
static constexpr int rawradsjksdfhksldjf = sizeof(guild_text_message);

struct dm_message :partial_message {
	user author;
	std::vector<user> mentions;
	
};

void from_json(const nlohmann::json& json, partial_message& msg);

struct msg_update {
	snowflake author_id;
	snowflake id;
	snowflake channel_id;
	std::string content;
	timestamp timestamp;
	//std::optional<::timestamp> edited_timestamp;
	bool tts = false;
	std::optional<bool> mention_everyone = false;

	std::vector<attachment> attachments;
	std::vector<reaction> reactions;
	friend struct internal_shard;
	friend void from_json(const nlohmann::json& json, msg_update& msg);
};

struct guild_msg_update :msg_update {

	
	std::optional<guild_member> author;
	std::optional<std::vector<snowflake>> mention_role_ids;
	std::optional<std::vector<guild_member>> mentions;
	
	friend struct internal_shard;
};

struct dm_msg_update :msg_update {
	std::optional<user> author;
	std::optional<std::vector<user>> mentions;
	
	friend struct internal_shard;
};

void from_json(const nlohmann::json& json, msg_update& msg);

struct msg_update_access {
	//TODO, maybe
	static void update_msg(guild_text_message& msg, guild_msg_update& update) { }

	static void update_msg(dm_message&, dm_msg_update&) { }
};
};
