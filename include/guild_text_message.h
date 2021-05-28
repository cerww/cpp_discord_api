#pragma once
#include "partial_message.h"

struct guild_message_base:partial_message {
	const Guild& guild()const noexcept {
		return m_channel->guild();
	}

	const text_channel& channel()const noexcept {
		return *m_channel;
	}
protected:
	ref_count_ptr<text_channel> m_channel = nullptr;
	friend struct shard;
};

struct referenced_guild_message :guild_message_base,ref_counted {

	std::span<const snowflake> mention_roles_ids() const noexcept {
		return m_mention_roles_ids;
	}

	std::span<const user> mentions() const noexcept {
		return m_mentions;
	}

	auto mention_roles() const noexcept {
		return m_mention_roles_ids | ranges::views::transform(hof::map_with(guild().roles()));
	}

	const user& author()const noexcept {
		return m_author;
	}
	
private:
	usually_empty_vector<snowflake> m_mention_roles_ids;
	usually_empty_vector<user> m_mentions;
	//ref_count_ptr<text_channel> m_channel = nullptr;
	user m_author;

	friend struct shard;
};

struct guild_text_message :guild_message_base {
	const guild_member& author() const noexcept;

	std::span<const snowflake> mention_roles_ids() const noexcept {
		return m_mention_roles_ids;
	}

	std::span<const guild_member> mentions() const noexcept {
		return m_mentions;
	}

	auto mention_roles() const noexcept {
		return m_mention_roles_ids | ranges::views::transform(hof::map_with(guild().roles()));
	}

	optional_ref<const referenced_guild_message> referenced_message()const {
		if (m_referenced_message) {
			return *m_referenced_message;
		}
		else {
			return std::nullopt;
		}
	}


private:
	guild_member m_author;
	usually_empty_vector<snowflake> m_mention_roles_ids;
	std::vector<guild_member> m_mentions;
	
	//ref_count_ptr<text_channel> m_channel = nullptr;

	ref_count_ptr<referenced_guild_message> m_referenced_message;

	friend struct internal_shard;
	friend struct shard;
	friend struct msg_update_access;
};

static constexpr int asueoasdasdhdasd = sizeof(std::vector<snowflake>);

static constexpr int rawradsjksdfhksldjfa = sizeof(guild_member);
static constexpr int rawradsjksdfhksldjf = sizeof(guild_text_message);

struct guild_msg_update :msg_update {
	const std::optional<guild_member>& author() const noexcept;

	std::optional<std::span<const snowflake>> mention_role_ids() const noexcept {
		return m_mention_role_ids;
	};

	std::optional<std::span<const guild_member>> mentions() const noexcept {
		return m_mentions;
	};

	auto mention_roles() const noexcept {
		//these could've been done with monanadic interface ;-;
		//return m_mention_roles.transform(ranges::views::indirect);
		using return_type = decltype(std::optional(m_mention_roles.value() | ranges::views::indirect));
		if (m_mention_roles.has_value()) {
			return std::optional(m_mention_roles.value() | ranges::views::indirect);
		}
		else {
			return return_type(std::nullopt);
		}
	};

	const text_channel& channel() const noexcept;

	const Guild& guild() const noexcept {
		return *m_guild;
	}

private:
	std::optional<guild_member> m_author;
	std::optional<std::vector<snowflake>> m_mention_role_ids;
	std::optional<std::vector<guild_member>> m_mentions;
	std::optional<std::vector<const guild_role*>> m_mention_roles;
	ref_count_ptr<text_channel> m_channel = nullptr;
	ref_count_ptr<Guild> m_guild = nullptr;
	friend struct internal_shard;
};

