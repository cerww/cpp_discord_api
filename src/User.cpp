#include "User.h"

snowflake user::id() const noexcept {
	return m_id;
}

bool user::is_bot() const noexcept {
	return m_bot;
}

std::string_view user::username() const noexcept {
	return std::string_view(m_username.data(), m_username.size());
}

std::optional<int> user::discriminator() const noexcept {
	if (m_discriminator == 10000) {
		return std::nullopt;
	} else {
		return m_discriminator;
	}
}

void to_json(nlohmann::json& json, const user& other) {
	json["id"] = other.id();
	json["username"] = other.username();
	json["bot"] = other.is_bot();	
	if(other.discriminator()) {
		json["discriminator"] = std::to_string(other.discriminator().value());
	}else {
		json["discriminator"] = "0";
	}
}

void from_json(const nlohmann::json& json, user& other) {
	other.m_id = json["id"].get<snowflake>();
	other.m_username = json["username"].get<folly::fbstring>();
	//other.m_username = json["username"].get<folly::fbstring>();
	other.m_bot = json.value("bot", false);
	other.m_discriminator = std::stoi(json.value("discriminator", "10000"));
	const auto& a = json["avatar"];
	if (a.is_null()) {
		other.m_avatar = "";
	} else {
		other.m_avatar = a.get<folly::fbstring>();
	}
	other.m_flags = json.value("flags", 0);
	other.m_public_flags = json.value("flags", 0);
	other.m_premium_type = json.value("flags", 0);
	other.m_system = json.value("system", false);
}
