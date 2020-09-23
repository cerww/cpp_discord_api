#pragma once
#include "partial_message.h"

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

constexpr int asjdihadasdasda = sizeof(dm_message);

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

