#include "permision_overwrite.h"



std::string overwrite_type_to_string(const overwrite_type e) {
	if (e == overwrite_type::role) return "role";
	if (e == overwrite_type::member) return "member";
	throw std::runtime_error("invalid enum");
}

void to_json(nlohmann::json& json, const permission_overwrite& data) {
	json["id"] = data.m_id;
	json["type"] = overwrite_type_to_string(data.m_type);
	json["allow"] = data.m_allow;
	json["deny"] = data.m_deny;
}

void from_json(const nlohmann::json& json, permission_overwrite& data) {
	data.m_id = json["id"].get<snowflake>();
	data.m_type = string_to_overwrite_type(json["type"].get<std::string>());
	data.m_allow = json["allow"].get<size_t>();
	data.m_deny = json["deny"].get<size_t>();
}
