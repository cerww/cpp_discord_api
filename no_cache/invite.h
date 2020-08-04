#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include "snowflake.h"
#include "user.h"

namespace cacheless {
namespace invite_internal {

	struct partial_guild {
		snowflake id;
		std::string name;
		std::string splash;
		std::string icon;
	};

	struct partial_channel {
		snowflake id;
		std::string name;
		int type = 0;
	};

	inline void from_json(const nlohmann::json& json, partial_guild& g) {
		g.id = json["id"].get<snowflake>();
		g.name = json["name"].get<std::string>();
		g.splash = json["splash"].get<std::string>();
		g.icon = json["icon"].get<std::string>();
	}

	inline void from_json(const nlohmann::json& json, partial_channel& g) {
		g.id = json["id"].get<snowflake>();
		g.name = json["name"].get<std::string>();
		g.type = json["type"].get<int>();
	}

}

struct invite {
	std::string code;
	invite_internal::partial_guild guild;
	invite_internal::partial_channel channel;
	friend void from_json(const nlohmann::json& j, invite& i);
};


struct invite_metadata {
	std::optional<user> user = std::nullopt;
	int uses = 0;
	int max_uses = 0;
	friend void from_json(const nlohmann::json& j, invite_metadata& stuff);
};

inline void from_json(const nlohmann::json& j, invite& i) {
	i.code = j["code"].get<std::string>();
	i.guild = j["guild"].get<invite_internal::partial_guild>();
	i.channel = j["channel"].get<invite_internal::partial_channel>();
}

inline void from_json(const nlohmann::json& j, invite_metadata& stuff) {
	stuff.max_uses = j["max_uses"].get<int>();
}
}
