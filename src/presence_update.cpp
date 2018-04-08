#include "presence_update.h"
const std::string& activity::name() const noexcept { return m_name; }
activity_type activity::type() const noexcept { return m_type; }
const std::optional<std::string>& activity::url() const noexcept { return m_url; }

void from_json(const nlohmann::json& json, activity& thing) {
	thing.m_name = json["name"].get<std::string>();
	thing.m_type = json["type"].get<activity_type>();
}

snowflake partial_presence_update::id() const noexcept { return m_id; }
Status partial_presence_update::status() const noexcept { return m_status; }
std::optional<activity>& partial_presence_update::game() noexcept { return m_game; }
const std::optional<activity>& partial_presence_update::game() const noexcept { return m_game; }

void from_json(const nlohmann::json& json, partial_presence_update& thing) {
	thing.m_id = json["user"]["id"].get<snowflake>();
	thing.m_status = json["status"].get<Status>();
	auto t = json["game"];
	if (!t.is_null()) thing.m_game.emplace(t.get<activity>());
}
