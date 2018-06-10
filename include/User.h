#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include "constant_stuffs.h"
#include "snowflake.h"

class user{
public:
	snowflake id() const noexcept;
	bool is_bot() const noexcept;

	const std::string& username() const noexcept;

	int discriminator() const noexcept;
	Status status() const noexcept;
private:
	snowflake m_id;
	std::string m_username = "";
	int m_discriminator = 0;	
	bool m_bot = false;
	Status m_status = Status::idle;
	std::string m_game;
	friend class shard;
	friend void from_json(const nlohmann::json&, user& other);
};

void to_json(nlohmann::json& json, const user& other);

void from_json(const nlohmann::json& json, user& other);
