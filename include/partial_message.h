#pragma once
#include <string>
#include <vector>
#include "guild_member.h"
#include "attachment.h"
#include "reaction.h"
#include <optional>
#include <range/v3/all.hpp>
#include <span>

struct dm_channel;
struct text_channel;
struct Guild;

struct partial_message {
	std::string_view content() const noexcept;;
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
	std::string m_content;
	timestamp m_timestamp;
	std::optional<timestamp> m_edited_timestamp;
	bool m_tts = false;
	bool m_mention_everyone = false;

	std::vector<attachment> m_attachments;
	std::vector<reaction> m_reactions;

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

	auto mentions() const noexcept {
		return m_mentions | ranges::views::indirect;
	};

	auto mention_roles() const noexcept {
		return m_mention_roles | ranges::views::indirect;
	};

private:
	std::vector<snowflake> m_mention_roles_ids;
	std::vector<const guild_member*> m_mentions;
	std::vector<const guild_role*> m_mention_roles;
	guild_member* m_author = nullptr;
	text_channel* m_channel = nullptr;
	friend struct internal_shard;
	friend struct msg_update_access;
};

static constexpr int rawradsjksdfhksldjf = sizeof(guild_text_message);

struct dm_message :partial_message {
	const user& author() const noexcept;

	auto mentions() const noexcept {
		return m_mentions | ranges::views::indirect;
	};

	const dm_channel& channel() const noexcept;
private:
	user* m_author = nullptr;
	std::vector<user*> m_mentions;
	dm_channel* m_channel = nullptr;
	friend struct internal_shard;
	friend struct msg_update_access;
};

void from_json(const nlohmann::json& json, partial_message& msg);

struct msg_update {
	snowflake id() const noexcept;
	snowflake channel_id() const noexcept;
	std::string_view content() const noexcept;
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
	friend struct internal_shard;
	friend void from_json(const nlohmann::json& json, msg_update& msg);
};

struct guild_msg_update :msg_update {
	const guild_member& author() const noexcept;

	std::span<const snowflake> mention_role_ids() const noexcept {
		return m_mention_role_ids;
	};

	auto mentions() const noexcept {
		return m_mentions | ranges::views::indirect;
	};

	auto mention_roles() const noexcept {
		return m_mention_roles | ranges::views::indirect;
	};

	const text_channel& channel() const noexcept;

private:
	guild_member* m_author = nullptr;
	std::vector<snowflake> m_mention_role_ids;
	std::vector<const guild_member*> m_mentions;
	std::vector<const guild_role*> m_mention_roles;
	text_channel* m_channel = nullptr;
	friend struct internal_shard;
};

struct dm_msg_update :msg_update {
	const user& author() const;

	auto mentions() const noexcept {
		return m_mentions | ranges::views::indirect;
	};

	const dm_channel& channel() const noexcept;
private:
	user* m_author = nullptr;
	std::vector<const user*> m_mentions;
	dm_channel* m_channel = nullptr;
	friend struct internal_shard;
};

void from_json(const nlohmann::json& json, msg_update& msg);

struct msg_update_access {
	//TODO
	static void update_msg(guild_text_message& msg, guild_msg_update& update) { }

	static void update_msg(dm_message&, dm_msg_update&) { }
};
