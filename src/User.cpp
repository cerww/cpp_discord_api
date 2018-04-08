#include "User.h"
snowflake User::id() const noexcept { return m_id; }
bool User::is_bot() const noexcept { return m_bot; }

const std::string& User::username() const noexcept {
	return m_username;
}

int User::discriminator() const noexcept {
	return m_discriminator;
}

Status User::status() const noexcept { return m_status; }

void to_json(nlohmann::json& json, const User& other) {
	json["id"] = std::to_string(other.id().val);
	json["username"] = other.username();
	json["bot"] = other.is_bot();
	json["discriminator"] = other.discriminator();
}

void from_json(const nlohmann::json& json, User& other) {
	other.m_id = json["id"].get<snowflake>();
	other.m_username = json["username"].get<std::string>();
	other.m_bot = json.value("bot", false);
	other.m_discriminator = std::stoi(json["discriminator"].get<std::string>());
}

