#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "permision_overwrite.h"

namespace cacheless {

struct internal_shard;

struct partial_channel {
	snowflake id;
	std::string name;

	friend struct internal_shard;
	friend void from_json(const nlohmann::json& json, partial_channel& channel);
};

inline void to_json(nlohmann::json& json, const partial_channel& ch) {
	json["id"] = ch.id;
	json["name"] = ch.name;
}

inline void from_json(const nlohmann::json& json, partial_channel& channel) {
	channel.id = json["id"].get<snowflake>();
	channel.name = json.value("name", std::string());
}

};
