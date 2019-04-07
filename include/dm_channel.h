#pragma once
#include "partial_channel.h"
#include "user.h"
#include "timestamp.h"
#include <nlohmann/json.hpp>
#include "partial_message.h"
#include <deque>
#include "item_cache.h"

struct dm_channel: partial_channel{
	snowflake last_message_id() const noexcept;

	auto recipients() const noexcept {
		return m_recipients | ranges::view::all;
	};
	timestamp last_pin_timestamp() const noexcept;

	auto msg_cache() const noexcept {
		return m_msg_cache.data() | ranges::view::all;
	};
private:
	snowflake m_last_message_id;
	ref_stable_map<snowflake, user> m_recipients;
	timestamp m_last_pin_timestamp;

	dynamic_item_cache<dm_message> m_msg_cache = dynamic_item_cache<dm_message>(15,false);

	dm_message& m_add_msg(dm_message msg);

	friend void from_json(const nlohmann::json& json, dm_channel& channel);
	friend struct shard;
};

void to_json(nlohmann::json& json, const dm_channel& channel);


void from_json(const nlohmann::json& json, dm_channel& channel);
