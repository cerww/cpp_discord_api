#include "User.h"

namespace cacheless {


void to_json(nlohmann::json& json, const user& other) {
	json["id"] = std::to_string(other.id.val);
	json["username"] = other.username;
	json["bot"] = other.bot;
	json["discriminator"] = other.discriminator;
}

void from_json(const nlohmann::json& json, user& other) {
	other.id = json["id"].get<snowflake>();
	other.username = json["username"].get<std::string>();
	other.bot = json.value("bot", false);
	other.discriminator = std::stoi(json.value("discriminator", "-1"));
	const auto& a = json["avatar"];
	if (a.is_null()) {
		other.avatar = "";
	} else {
		other.avatar = json["avatar"];
	}
}
}
