#include "permision_overwrite.h"


void to_json(nlohmann::json& json, const permission_overwrite& data) {
	json["id"] = data.m_id;
	json["type"] = data.m_type;
	json["allow"] = data.m_allow;
	json["deny"] = data.m_deny;
}

void from_json(const nlohmann::json& json, permission_overwrite& data) {
	data.m_id = json["id"].get<snowflake>();
	//data.m_type = string_to_overwrite_type(json["type"].get<std::string>());
	data.m_type = json["type"].get<overwrite_type>();
	data.m_allow = json["allow"].get<permission>();
	data.m_deny = json["deny"].get<permission>();
}
