#include "guild_role.h"

namespace cacheless {

void to_json(nlohmann::json& json, const guild_role& r) {
	json["position"] = r.position;
	json["permissions"] = r.permissions.data();
	json["id"] = r.id;
	json["hoist"] = r.hoist;
	json["mentionable"] = r.mentionable;
	json["name"] = r.name;
	json["managed"] = r.managed;
}

void from_json(const nlohmann::json& json, guild_role& other) {
	if (json.is_null()) {
		return;//why would role be null ;-;
	}
	other.id = json["id"].get<snowflake>();
	other.position = json["position"].get<int>();
	other.permissions = json["permissions"].get<permission>();
	other.hoist = json["hoist"].get<bool>();
	other.managed = json["managed"].get<bool>();
	other.name = json["name"].get<std::string>();
	other.mentionable = json["mentionable"].get<bool>();
}
};
