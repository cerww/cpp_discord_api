#pragma once
#include "partial_message.h"

struct referenced_dm_message:referenced_message {



	std::span<const user> mentions() const noexcept {
		return m_mentions;
	}

	const dm_channel& channel() const noexcept {
		return *m_channel;
	}

	
private:
	dm_channel* m_channel = nullptr;
	usually_empty_vector<user> m_mentions;
	friend struct shard;
};

struct dm_message :partial_message {
	const user& author() const noexcept;

	std::span<const user> mentions() const noexcept {
		return m_mentions;
	}

	const dm_channel& channel() const noexcept;

	std::optional<referenced_dm_message> referenced_message()const {
		if (m_referenced_message) {
			return *m_referenced_message;
		}else {
			return std::nullopt;
		}
	}

private:
	user m_author;
	usually_empty_vector<user> m_mentions;
	ref_count_ptr<::referenced_dm_message> m_referenced_message = nullptr;
	dm_channel* m_channel = nullptr;
	friend struct internal_shard;
	friend struct shard;	
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

