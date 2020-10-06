#pragma once
#include "text_channel.h"
#include "partial_guild_member.h"
#include "partial_message.h"
#include "voice_channel.h"
#include "guild.h"
#include "emoji.h"
#include "channel_catagory.h"
#include "dm_channel.h"
#include "guild_role.h"
#include "guild_member_update.h"
#include "unavailable_guild.h"
#include <nlohmann/json.hpp>
#include "../common/common/optional_from_json.h"

namespace cacheless::events {

struct presence_update_event {
	//user user;
	snowflake user_id;
	std::optional<activity> game;
	snowflake guild_id;
	Status status;
	std::vector<activity> activities;
	client_status client_status;

};

inline void from_json(const nlohmann::json& json, presence_update_event& e) {
	//json["user"].get_to(e.user);
	json["user"]["id"].get_to(e.user_id);
	json["game"].get_to(e.game);
	json["guild_id"].get_to(e.guild_id);
	e.status = string_to_status(json["status"].get<std::string>());
	json["activities"].get_to(e.activities);
	json["client_status"].get_to(e.client_status);


}

struct text_channel_create {
	text_channel channel;
};

struct voice_channel_create {
	voice_channel channel;
};

struct dm_channel_create {
	dm_channel channel;
};

struct channel_catagory_create {
	channel_catagory channel;
};

struct text_channel_update {
	text_channel channel;
};

struct voice_channel_update {
	voice_channel channel;
};

struct dm_channel_update {
	dm_channel channel;
};

struct channel_catagory_update {
	channel_catagory channel;
};

struct text_channel_delete {
	text_channel channel;
};

struct voice_channel_delete {
	voice_channel channel;
};

struct dm_channel_delete {
	dm_channel channel;
};

struct channel_catagory_delete {
	channel_catagory channel;
};

struct channel_pins_update {
	std::optional<snowflake> guild_id;
	snowflake channel_id;
	timestamp last_pin_timestamp;
};

struct guild_create {
	Guild guild;
};

struct guild_update {
	partial_guild guild;
};

struct guild_delete {
	unavailable_guild guild;
};

struct guild_ban_add {
	snowflake guild_id;
	user user;
};

struct guild_ban_remove {
	snowflake guild_id;
	user user;
};

struct guild_emoji_update {
	snowflake guild_id;
	std::vector<emoji> emojis;
};

struct guild_integration_update {
	snowflake guild_id;
};

struct guild_member_add {
	snowflake guild_id;
	guild_member member;
};

struct guild_member_remove {
	snowflake guild_id;
	user member;
};

struct guild_member_update {
	user user;
	std::vector<guild_role> roles;
	std::optional<std::string> nick;
	snowflake guild_id;
	std::optional<timestamp> premium_since;
};

struct guild_members_chunk {
	snowflake guild_id;
	std::vector<guild_member> members;
	int chunk_index;
	int chunk_count;
	std::vector<presence_update_event> presences;
};

struct guild_role_create {
	snowflake guild_id;
	guild_role role;
};

struct guild_role_update {
	snowflake guild_id;
	guild_role role;
};

struct guild_role_delete {
	snowflake guild_id;
	snowflake role_id;
};

struct invite_create {
	snowflake channel_id;
	std::string code;
	timestamp created_at;
	std::optional<snowflake> guild_id;
	user inviter;
	int max_age;
	int max_uses;
	nlohmann::json target_user;//wat
	int target_user_type;
	bool temporary;
	int uses;
	//NLOHMANN_DEFINE_TYPE_INTRUSIVE(invite_create, channel_id, code, created_at, inviter, max_age, max_uses, target_user_type, temporary, uses);


};

struct invite_delete {
	snowflake channel_id;
	std::optional<snowflake> guild_id;
	std::string code;
};

struct guild_message_create {
	guild_text_message msg;

};

struct dm_message_create {
	dm_message msg;
};

struct guild_message_delete {
	snowflake id;
	snowflake channel_id;
	snowflake guild_id;
};

struct dm_message_delete {
	snowflake id;
	snowflake channel_id;
};

struct guild_message_delete_bulk {
	std::vector<snowflake> ids;
	snowflake channel_id;
	snowflake guild_id;
};

struct dm_message_delete_bulk {
	std::vector<snowflake> ids;
	snowflake channel_id;
};

struct guild_message_reaction_add {
	snowflake user_id;
	snowflake channel_id;
	snowflake guild_id;
	snowflake message_id;
	guild_member member;
	partial_emoji emoji;
};


struct guild_message_reaction_remove {
	snowflake user_id;
	snowflake channel_id;
	snowflake guild_id;
	snowflake message_id;
	partial_emoji emoji;
};

struct dm_message_reaction_add {
	snowflake user_id;
	snowflake channel_id;
	snowflake message_id;
	guild_member member;
	partial_emoji emoji;
};


struct dm_message_reaction_remove {
	snowflake user_id;
	snowflake channel_id;
	snowflake message_id;
	partial_emoji emoji;
};

struct guild_message_reaction_remove_all {
	snowflake channel_id;
	snowflake message_id;
	snowflake guild_id;
};

struct dm_message_reaction_remove_all {
	snowflake channel_id;
	snowflake message_id;
};

struct guild_typing_start {
	snowflake channel_id;
	snowflake guild_id;
	snowflake user_id;
	int timestamp = 0;
	guild_member member;
};

struct dm_typing_start {
	snowflake channel_id;
	snowflake user_id;
	int timestamp = 0;
};

struct guild_voice_state_update {
	voice_state2 state;	
};

struct dm_voice_state_update {
	voice_state state;
};

struct guild_message_update {
	guild_msg_update message;
};

struct dm_message_update {
	dm_msg_update message;
};

}
