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
#include "../common/lol_wat_vector.h"


enum struct message_type:uint8_t {
	DEFAULT= 0,
	RECIPIENT_ADD = 1,
	RECIPIENT_REMOVE =2,
	CALL =3,
	CHANNEL_NAME_CHANGE =4,
	CHANNEL_ICON_CHANGE =5,
	CHANNEL_PINNED_MESSAGE =6,
	GUILD_MEMBER_JOIN =7,
	USER_PREMIUM_GUILD_SUBSCRIPTION =8,
	USER_PREMIUM_GUILD_SUBSCRIPTION_TIER_1 =9,
	USER_PREMIUM_GUILD_SUBSCRIPTION_TIER_2 =10,
	USER_PREMIUM_GUILD_SUBSCRIPTION_TIER_3 =11,
	CHANNEL_FOLLOW_ADD =12,
	GUILD_DISCOVERY_DISQUALIFIED= 14,
	GUILD_DISCOVERY_REQUALIFIED =15,
	GUILD_DISCOVERY_GRACE_PERIOD_INITIAL_WARNING= 16,
	GUILD_DISCOVERY_GRACE_PERIOD_FINAL_WARNING =17,
	THREAD_CREATED =18,
	REPLY =19,
	APPLICATION_COMMAND =20,
	THREAD_STARTER_MESSAGE =21,
	GUILD_INVITE_REMINDER =22
};

struct partial_message {

	std::string_view content() const noexcept {
		return std::string_view(m_content.data(), m_content.size());
	}

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

	void force_heap_allocated() {//TODO remove this sometime
		m_content.reserve(sizeof(m_content) + 1); // NOLINT
	}

	message_type type()const noexcept {
		
	}

private:
	snowflake m_author_id;
	snowflake m_id;
	snowflake m_channel_id;

	lol_wat_vector<embed> m_embeds;
	lol_wat_vector<attachment> m_attachments;
	lol_wat_vector<reaction> m_reactions;

	folly::fbstring m_content;
	timestamp m_timestamp;
	bool m_tts = false;
	bool m_mention_everyone = false;
	message_type m_type = message_type::DEFAULT;

	

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
