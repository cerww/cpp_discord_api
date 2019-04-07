#pragma once
#include "snowflake.h"
#include "discord_enums.h"
#include <optional>

enum struct activity_type{
	game,
	streaming,
	listening
};

struct activity{
	std::string_view name() const noexcept;
	activity_type type() const noexcept;
	const std::optional<std::string>& url() const noexcept;
private:
	std::string m_name;
	activity_type m_type = activity_type::game;
	std::optional<std::string> m_url;
	friend void from_json(const nlohmann::json& json, activity& thing);
};

void from_json(const nlohmann::json& json, activity& thing);

struct partial_presence_update{
	snowflake id() const noexcept;
	Status status() const noexcept;
	const std::optional<activity>& game() const noexcept;
private:
	snowflake m_id;
	Status m_status = Status::dnd;
	std::optional<activity> m_game;
	friend void from_json(const nlohmann::json&, partial_presence_update&);
};


void from_json(const nlohmann::json& json, partial_presence_update& thing);
