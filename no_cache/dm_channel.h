#pragma once
#include "partial_channel.h"
#include "User.h"
#include "timestamp.h"
#include <nlohmann/json.hpp>
#include "partial_message.h"
#include <deque>
#include "item_cache.h"
#include <span>
#include "../common/ref_stable_map.h"

namespace cacheless {

struct dm_channel :partial_channel {
	snowflake last_message_id;
	ref_stable_map<snowflake, user> recipients;
	timestamp last_pin_timestamp;


	friend void from_json(const nlohmann::json& json, dm_channel& channel);
	friend struct internal_shard;
};

void to_json(nlohmann::json& json, const dm_channel& channel);


void from_json(const nlohmann::json& json, dm_channel& channel);
}
