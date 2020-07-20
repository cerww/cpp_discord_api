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

struct dm_channel;
struct text_channel;
struct Guild;

struct partial_message {
	std::string_view content() const noexcept { return m_content; }
	snowflake id() const noexcept;
	snowflake author_id() const noexcept;
	snowflake channel_id() const noexcept;
	bool tts() const noexcept;
	bool mention_everyone() const noexcept;

	std::span<const reaction> reactions() const noexcept {
		return m_reactions;
	};

	std::span<const attachment> attachments() const noexcept {
		return m_attachments;
	};

private:
	snowflake m_author_id;
	snowflake m_id;
	snowflake m_channel_id;
	std::vector<embed> m_embeds;
	std::vector<attachment> m_attachments;
	std::vector<reaction> m_reactions;
	std::string m_content;
	std::optional<timestamp> m_edited_timestamp;
	timestamp m_timestamp;
	bool m_tts = false;
	bool m_mention_everyone = false;


	friend struct client;
	friend struct internal_shard;
	friend void from_json(const nlohmann::json& json, partial_message& msg);
	friend struct msg_update_access;
};


struct guild_text_message :partial_message {
	
	const text_channel& channel() const noexcept;;
	const Guild& guild() const noexcept;
	const guild_member& author() const noexcept;

	std::span<const snowflake> mention_roles_ids() const noexcept {
		return m_mention_roles_ids;
	};

	std::span<const guild_member> mentions() const noexcept {
		return m_mentions;
	}	

	auto mention_roles() const noexcept {
		return m_mention_roles | ranges::views::indirect;
	}

private:
	guild_member m_author;
	std::vector<snowflake> m_mention_roles_ids;
	std::vector<guild_member> m_mentions;
	std::vector<const guild_role*> m_mention_roles;
	text_channel* m_channel = nullptr;
	friend struct internal_shard;
	friend struct msg_update_access;
};

static constexpr int asueohdasd = sizeof(std::vector<snowflake>);

static constexpr int rawradsjksdfhksldjfa = sizeof(guild_member);
static constexpr int rawradsjksdfhksldjf = sizeof(guild_text_message);

struct dm_message :partial_message {
	const user& author() const noexcept;

	std::span<const user> mentions() const noexcept {
		return m_mentions;
	}

	const dm_channel& channel() const noexcept;
private:
	user m_author;
	std::vector<user> m_mentions;
	dm_channel* m_channel = nullptr;
	friend struct internal_shard;
	friend struct msg_update_access;
};

void from_json(const nlohmann::json& json, partial_message& msg);

struct msg_update {
	snowflake id() const noexcept;
	snowflake channel_id() const noexcept;
	std::optional<std::string_view> content() const noexcept;
	
	std::optional<bool> mention_everyone()const noexcept {
		return m_mention_everyone;
	}
	
private:
	snowflake m_author_id;
	snowflake m_id;
	snowflake m_channel_id;
	std::string m_content;
	timestamp m_timestamp;
	std::optional<timestamp> m_edited_timestamp;
	bool m_tts = false;
	std::optional<bool> m_mention_everyone = false;

	std::vector<attachment> m_attachments;
	std::vector<reaction> m_reactions;
	friend struct internal_shard;
	friend void from_json(const nlohmann::json& json, msg_update& msg);
};

struct guild_msg_update :msg_update {
	const std::optional<guild_member>& author() const noexcept;

	std::optional<std::span<const snowflake>> mention_role_ids() const noexcept {
		return m_mention_role_ids;
	};

	std::optional<std::span<const guild_member>> mentions() const noexcept {
		return m_mentions;
	};

	auto mention_roles() const noexcept
			
	{
		//these could've been done with monanadic interface
		using return_type = decltype(std::optional(m_mention_roles.value() | ranges::views::indirect));
		if(m_mention_roles.has_value()) {
			return std::optional(m_mention_roles.value() | ranges::views::indirect);
		}else {
			return return_type(std::nullopt);
		}
	};

	const text_channel& channel() const noexcept;

	const Guild& guild()const noexcept {
		return *m_guild;
	}
	
private:
	std::optional<guild_member> m_author;
	std::optional<std::vector<snowflake>> m_mention_role_ids;
	std::optional<std::vector<guild_member>> m_mentions;
	std::optional<std::vector<const guild_role*>> m_mention_roles;
	text_channel* m_channel = nullptr;
	Guild* m_guild = nullptr;
	friend struct internal_shard;
};

struct dm_msg_update :msg_update {
	const std::optional<user>& author() const;

	std::optional<std::span<const user>> mentions() const noexcept {		
		return m_mentions;
	};

	const dm_channel& channel() const noexcept;
private:
	std::optional<user> m_author;
	std::optional<std::vector<user>> m_mentions;
	dm_channel* m_channel = nullptr;
	friend struct internal_shard;
};

void from_json(const nlohmann::json& json, msg_update& msg);

struct msg_update_access {
	//TODO, maybe
	static void update_msg(guild_text_message& msg, guild_msg_update& update) { }

	static void update_msg(dm_message&, dm_msg_update&) { }
};
