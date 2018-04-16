#pragma once
#include <string>
#include <vector>
#include "guildMember.h"
#include "attachment.h"
#include "reaction.h"
#include <optional>

class dm_channel;
class text_channel;
class Guild;

class partial_message{
public:
	const std::string& content() const noexcept;;
	snowflake id() const noexcept;
	snowflake author_id() const noexcept;
	snowflake channel_id() const noexcept;
	bool tts() const noexcept;
	bool mention_everyone() const noexcept;
	const std::vector<reaction>& reactions() const noexcept;
	const std::vector<attachment>& attachments() const noexcept;
private:
	snowflake m_author_id;
	snowflake m_id;
	snowflake m_channel_id;
	//sender_t m_author;
	std::string m_content;
	timestamp m_timestamp;
	//std::optional<timestamp> m_edited_timestamp;
	bool m_tts = false;	
	bool m_mention_everyone = false;
	
	std::vector<attachment> m_attachments;
	std::vector<reaction> m_reactions;

	friend class client;
	friend class shard;
	friend void from_json(const nlohmann::json& json, partial_message& msg);
};


class guild_text_message:public partial_message{
public:
	text_channel& channel() noexcept;;
	const text_channel& channel() const noexcept;;
	Guild& guild()noexcept;
	const Guild& guild()const noexcept;
	guild_member& author() noexcept;
	const guild_member& author() const noexcept;
	const std::vector<snowflake>& mention_roles() const noexcept;
	const std::vector<guild_member*>& mentions() const noexcept;
private:
	std::vector<snowflake> m_mention_roles;
	std::vector<guild_member*> m_mentions;
	guild_member* m_author = nullptr;
	text_channel* m_channel = nullptr;
	friend class shard;
};

class dm_message:public partial_message {
public:
	User& author() noexcept;
	const User& author() const noexcept;
	const std::vector<User*>& mentions() const noexcept;;
	dm_channel& channel() noexcept;
	const dm_channel& channel() const noexcept;
private:
	User* m_author = nullptr;
	std::vector<User*> m_mentions;
	dm_channel * m_channel = nullptr;
	friend class shard;
};

void from_json(const nlohmann::json& json, partial_message& msg);

class msg_update{
public:
	snowflake id() const noexcept;
	snowflake channel_id() const noexcept;
	const std::string& content() const noexcept;
private:
	snowflake m_author_id;
	snowflake m_id;
	snowflake m_channel_id;
	//sender_t m_author;
	std::string m_content;
	timestamp m_timestamp;
	//std::optional<timestamp> m_edited_timestamp;
	bool m_tts = false;
	bool m_mention_everyone = false;

	std::vector<attachment> m_attachments;
	std::vector<reaction> m_reactions;
	friend class shard;
	friend void from_json(const nlohmann::json& json, msg_update& msg);
};

class guild_msg_update:public msg_update{
public:

private:
	guild_member* m_author = nullptr;
	std::vector<snowflake> m_mention_roles;
	std::vector<guild_member*> m_mentions;
	text_channel* m_channel = nullptr;
	friend class shard;
};
class dm_msg_update:public msg_update{
public:
	User& author();
	const User& author() const;
private:
	User * m_author = nullptr;
	std::vector<User*> m_mentions;
	dm_channel * m_channel = nullptr;
	friend class shard;
};

void from_json(const nlohmann::json& json, msg_update& msg);


inline void update_msg(guild_text_message&,const guild_msg_update&) {
	
}

inline void update_msg(dm_message&, const dm_msg_update&) {

}
