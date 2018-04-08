#include "role.h"
int Role::position() const noexcept { return m_position; }
permission Role::permissions() const noexcept { return m_permissions; }
snowflake Role::id() const noexcept { return m_id; }
bool Role::managed() const noexcept { return m_managed; }
bool Role::mentionable() const noexcept { return m_mentionable; }
bool Role::hoist() const noexcept { return m_hoist; }
const std::string& Role::name() const noexcept { return m_name; }

void to_json(nlohmann::json& json, const Role& r) {
	json["position"] = r.position();
	json["permissions"] = r.permissions().data();
	json["id"] = r.id();
	json["hoist"] = r.hoist();
	json["mentionable"] = r.mentionable();
	json["name"] = r.name();
	json["managed"] = r.managed();
}

void from_json(const nlohmann::json& json, Role& other) {
	if (json.is_null()) return;
	other.m_id = json["id"].get<snowflake>();
	other.m_position = json["position"].get<int>();
	other.m_permissions.data() = json["permissions"].get<size_t>();
	other.m_hoist = json["hoist"].get<bool>();
	other.m_managed = json["managed"].get<bool>();
	other.m_name = json["name"].get<std::string>();
	other.m_mentionable = json["mentionable"].get<bool>();
}
