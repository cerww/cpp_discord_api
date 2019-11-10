#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include "discord_enums.h"
#include "snowflake.h"

struct user {
	snowflake id() const noexcept;
	bool is_bot() const noexcept;

	std::string_view username() const noexcept;

	int discriminator() const noexcept;
	Status status() const noexcept;

	std::string_view game_name() const noexcept { return m_game; }

private:
	snowflake m_id;
	std::string m_username = "";
	int m_discriminator = 0;
	bool m_bot = false;
	Status m_status = Status::online;
	std::string m_game;
	friend struct shard;
	friend void from_json(const nlohmann::json&, user& other);
};

void to_json(nlohmann::json& json, const user& other);

void from_json(const nlohmann::json& json, user& other);
