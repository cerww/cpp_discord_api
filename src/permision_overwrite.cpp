#include "permision_overwrite.h"



std::string overwrite_type_to_string(const overwrite_type e) {
	if (e == overwrite_type::role) return "role";
	if (e == overwrite_type::member) return "member";
	throw std::runtime_error("invalid enum");
}

void to_json(nlohmann::json& json, const permission_overwrite& data) {
	json["id"] = data.id;
	json["type"] = overwrite_type_to_string(data.type);
	json["allow"] = data.allow;
	json["deny"] = data.deny;
}

void from_json(const nlohmann::json& json, permission_overwrite& data) {
	data.id = json["id"].get<snowflake>();
	data.type = string_to_overwrite_type(json["type"].get<std::string>());
	data.allow = json["allow"].get<size_t>();
	data.deny = json["deny"].get<size_t>();
}
