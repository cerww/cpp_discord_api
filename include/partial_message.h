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
#include "guild.h"
#include "dm_channel.h"
#include "../common/higher_order_functions.h"


struct partial_message {

	std::string_view content() const noexcept { return m_content; }

	snowflake id() const noexcept;
	snowflake author_id() const noexcept;
	snowflake channel_id() const noexcept { return m_channel_id; }
	bool tts() const noexcept;
	bool mention_everyone() const noexcept;

	std::span<const reaction> reactions() const noexcept {
		return m_reactions;
	};

	std::span<const attachment> attachments() const noexcept {
		return m_attachments;
	};

	std::span<const embed> embeds() const noexcept {
		return m_embeds;
	}

	void force_heap_allocated() {
		m_content.reserve(sizeof(m_content) + 1); // NOLINT
	}

private:
	snowflake m_author_id;
	snowflake m_id;
	snowflake m_channel_id;
	std::vector<embed> m_embeds;
	std::vector<attachment> m_attachments;
	std::vector<reaction> m_reactions;
	std::string m_content;
	//std::optional<timestamp> m_edited_timestamp;
	timestamp m_timestamp;
	bool m_tts = false;
	bool m_mention_everyone = false;

	friend struct internal_shard;
	friend void from_json(const nlohmann::json& json, partial_message& msg);
	friend struct msg_update_access;
};

constexpr int asudhgasdasd = sizeof(partial_message);



void from_json(const nlohmann::json& json, partial_message& msg);

struct msg_update {
	snowflake id() const noexcept;
	snowflake channel_id() const noexcept;
	std::optional<std::string_view> content() const noexcept;

	std::optional<bool> mention_everyone() const noexcept {
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


void from_json(const nlohmann::json& json, msg_update& msg);

struct msg_update_access {
	//TODO, maybe
	//static void update_msg(guild_text_message& msg, guild_msg_update& update) { }

	//static void update_msg(dm_message&, dm_msg_update&) { }
};
