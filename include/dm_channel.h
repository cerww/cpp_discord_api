#pragma once
#include "partial_channel.h"
#include "User.h"
#include "timestamp.h"
#include <nlohmann/json.hpp>
#include <deque>
#include "../common/item_cache.h"
#include <span>

struct dm_channel :partial_channel {
	snowflake last_message_id() const noexcept;

	const auto& recipients() const noexcept {
		return m_recipients;
	};

	auto recipients_list() const noexcept {
		return m_recipients | ranges::views::values | ranges::views::all;
	};

	timestamp last_pin_timestamp() const noexcept;

	/*
	std::span<const dm_message> msg_cache() const noexcept {
		return m_msg_cache.data();
	};
	*/

private:
	snowflake m_last_message_id;
	ref_stable_map<snowflake, user> m_recipients;
	timestamp m_last_pin_timestamp;

	//dynamic_item_cache<dm_message> m_msg_cache = dynamic_item_cache<dm_message>(15, false);

	//dm_message& m_add_msg(dm_message msg);

	friend void from_json(const nlohmann::json& json, dm_channel& channel);
	friend struct internal_shard;
};

void to_json(nlohmann::json& json, const dm_channel& channel);


void from_json(const nlohmann::json& json, dm_channel& channel);
