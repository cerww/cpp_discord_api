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

<<<<<<< HEAD
	std::string to_mentionable_string() const {
		return "<@&" + std::to_string(m_id.val) + ">";
	}
	
private:
	snowflake m_id;
	std::string m_username = "";
	std::string m_game;
	Status m_status = Status::unknown;
	int16_t m_discriminator = 0;
	bool m_bot = false;
=======
private:
	snowflake m_id;
	std::string m_username = "";
	int m_discriminator = 0;
	bool m_bot = false;
	Status m_status = Status::unknown;
	std::string m_game;
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
	friend struct internal_shard;
	friend void from_json(const nlohmann::json&, user& other);
};

constexpr int a = sizeof(user);

void to_json(nlohmann::json& json, const user& other);

void from_json(const nlohmann::json& json, user& other);
