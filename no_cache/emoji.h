#pragma once
#include <nlohmann/json.hpp>
#include "snowflake.h"
#include <range/v3/core.hpp>
#include <span>
#include <fmt/core.h>
#include <fmt/format.h>

namespace cacheless {

struct partial_emoji {
	partial_emoji() = default;

	explicit partial_emoji(std::string name) :
		name(std::move(name)) {}

	std::string to_string() const {
		if (id== snowflake(0)) {
			return ":" + name+ ":";
		}
		if (animated) {
			return fmt::format("<a:{}:{}>", name, id);
		} else {
			return fmt::format("<:{}:{}>", name, id);
		}
	}

	std::string to_reaction_string() const {
		if (id== snowflake(0)) {
			return ":" + name + ":";
		}
		if (animated) {
			return fmt::format("a:{}:{}", name, id);
		} else {
			return fmt::format(":{}:{}", name, id);
		}
	}

	snowflake id = snowflake(0);
	std::string name;
	bool animated = false;
	
	friend void from_json(const nlohmann::json&, partial_emoji&);
};

struct emoji :partial_emoji {
	std::vector<snowflake> roles;
	snowflake user_id;
};

inline void from_json(const nlohmann::json& json, partial_emoji& e) {
	e.id = json.value("id", snowflake(0));
	e.name = json.value("name", std::string(""));
	e.animated = json.value("animated", false);
}

inline void from_json(const nlohmann::json& json, emoji& e) {
	from_json(json, static_cast<partial_emoji&>(e));
}
}
