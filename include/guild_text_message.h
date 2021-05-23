#pragma once
#include "partial_message.h"

struct guild_text_message :partial_message {
	const text_channel& channel() const noexcept;
	const Guild& guild() const noexcept;
	const guild_member& author() const noexcept;

	std::span<const snowflake> mention_roles_ids() const noexcept {
		return m_mention_roles_ids;
	};

	std::span<const guild_member> mentions() const noexcept {
		return m_mentions;
	}

	auto mention_roles() const noexcept {
		return m_mention_roles_ids | ranges::views::transform(hof::map_with(guild().roles()));
	}

private:
	guild_member m_author;
	lol_wat_vector<snowflake> m_mention_roles_ids;
	std::vector<guild_member> m_mentions;
	ref_count_ptr<text_channel> m_channel = nullptr;

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

