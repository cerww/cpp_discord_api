#pragma once
#include <nlohmann/json.hpp>
#include "emoji.h"

namespace cacheless {

struct reaction {
	int count = 0;
	bool me = false;
	partial_emoji emoji;

	friend void from_json(const nlohmann::json&, reaction&);
	friend struct internal_shard;
};

void from_json(const nlohmann::json& json, reaction& r);
};
