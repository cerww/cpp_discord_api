#include "User.h"

snowflake user::id() const noexcept { return m_id; }

bool user::is_bot() const noexcept { return m_bot; }

std::string_view user::username() const noexcept {
	return m_username;
}

int user::discriminator() const noexcept {
	return m_discriminator;
}

Status user::status() const noexcept { return m_status; }

void to_json(nlohmann::json& json, const user& other) {
	json["id"] = std::to_string(other.id().val);
	json["username"] = other.username();
	json["bot"] = other.is_bot();
	json["discriminator"] = other.discriminator();
}

void from_json(const nlohmann::json& json, user& other) {
	other.m_id = json["id"].get<snowflake>();
	other.m_username = json["username"].get<std::string>();
	other.m_bot = json.value("bot", false);
	other.m_discriminator = std::stoi(json.value("discriminator", "-1"));
}
