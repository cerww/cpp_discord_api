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
	std::string m_content;
	timestamp m_timestamp;
	std::optional<timestamp> m_edited_timestamp;
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
	const text_channel& channel() const noexcept;;
	const Guild& guild()const noexcept;
	const guild_member& author() const noexcept;
	const std::vector<snowflake>& mention_roles_ids() const noexcept;
	const std::vector<const guild_member*>& mentions() const noexcept;
	const std::vector<const guild_role*>& mention_roles()const noexcept;
private:
	std::vector<snowflake> m_mention_roles_ids;
	std::vector<const guild_member*> m_mentions;
	std::vector<const guild_role*> m_mention_roles;
	guild_member* m_author = nullptr;
	text_channel* m_channel = nullptr;
	friend class shard;
};

class dm_message:public partial_message {
public:
	const user& author() const noexcept;
	const std::vector<user*>& mentions() const noexcept;;
	const dm_channel& channel() const noexcept;
private:
	user* m_author = nullptr;
	std::vector<user*> m_mentions;
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
	std::string m_content;
	timestamp m_timestamp;
	std::optional<timestamp> m_edited_timestamp;
	bool m_tts = false;
	bool m_mention_everyone = false;

	std::vector<attachment> m_attachments;
	std::vector<reaction> m_reactions;
	friend class shard;
	friend void from_json(const nlohmann::json& json, msg_update& msg);
};

class guild_msg_update:public msg_update{
public:
	const guild_member& author()const noexcept;
	const std::vector<snowflake>& mention_role_ids()const noexcept;
	const std::vector<const guild_member*>& mentions()const noexcept;
	const std::vector<const guild_role*>& mention_roles()const noexcept;
	const text_channel& channel()const noexcept;

private:
	guild_member* m_author = nullptr;
	std::vector<snowflake> m_mention_role_ids;
	std::vector<const guild_member*> m_mentions;
	std::vector<const guild_role*> m_mention_roles;
	text_channel* m_channel = nullptr;
	friend class shard;
};

class dm_msg_update:public msg_update{
public:
	//user& author();
	const user& author() const;
	const std::vector<const user*>& mentions()const noexcept;
	const dm_channel& channel()const noexcept;
private:
	user* m_author = nullptr;
	std::vector<const user*> m_mentions;
	dm_channel * m_channel = nullptr;
	friend class shard;
};

void from_json(const nlohmann::json& json, msg_update& msg);

inline void update_msg(guild_text_message&,const guild_msg_update&) {
	
}

inline void update_msg(dm_message&, const dm_msg_update&) {

}
