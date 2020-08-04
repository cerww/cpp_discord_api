#pragma once
#include "snowflake.h"
#include <optional>
#include "User.h"
#include "timestamp.h"

namespace cacheless {
struct integration_account {

	std::string id;
	std::string name;
	friend void from_json(const nlohmann::json&, integration_account&);
};

inline void from_json(const nlohmann::json& json, integration_account& me) {
	me.id = json["id"].get<std::string>();
	me.name = json["name"].get<std::string>();
}

struct integration_expire_behavior {


private:

	friend void from_json(const nlohmann::json& json, integration_expire_behavior& me) {
		//wat
	}
};

struct partial_integration {
	snowflake id;
	std::string name;
	std::string type;
	integration_account account;

	friend void from_json(const nlohmann::json& json, partial_integration& me) {
		me.id = json["id"].get<snowflake>();
		me.name = json["name"].get<std::string>();
		me.type = json["type"].get<std::string>();
		me.account = json["account"].get<integration_account>();
	}
};

struct guild_integration :partial_integration {
	bool enabled = false;
	bool syncing = false;
	snowflake role_id;
	std::optional<bool> enable_emoticons;//wat optional<bool> ???
	integration_expire_behavior expire_behavior;
	int expire_grace_period = 0;
	cacheless::user user;
	timestamp synced_at;

	friend void from_json(const nlohmann::json& json, guild_integration& me) {
		me.enabled = json["enabled"].get<bool>();
		me.syncing = json["syncing"].get<bool>();
		me.role_id = json["role_id"].get<snowflake>();
		me.enable_emoticons = json.value("enable_emoticons", std::optional<bool>());
		me.expire_behavior = json["expire_behavior"].get<integration_expire_behavior>();
		me.expire_grace_period = json["expire_grace_period"].get<int>();
		me.user = json["user"].get<cacheless::user>();
		me.synced_at = json["synced_at"].get<timestamp>();
	}
};
};
