#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "snowflake.h"
#include "permission.h"
#include <fmt/format.h>


struct guild_role {
	snowflake id() const noexcept;;
	int position() const noexcept;
	const permission& permissions() const noexcept;
	bool managed() const noexcept;
	bool mentionable() const noexcept;
	bool hoist() const noexcept;
	std::string_view name() const noexcept;
	
	std::string to_mentionable_string()const {
		return "<@&" + std::to_string(m_id.val) + ">";
	}
	
private:
	snowflake m_id;
	std::string m_name;
	permission m_permissions;
	int m_position = 0;
	bool m_managed = false;
	bool m_mentionable = false;
	bool m_hoist = false;
	friend void from_json(const nlohmann::json& json, guild_role& other);
};

void to_json(nlohmann::json& json, const guild_role& r);

void from_json(const nlohmann::json& json, guild_role& other);

template<typename Char>
struct fmt::formatter<guild_role,Char>:fmt::formatter<std::string_view,Char> {
	
	template<typename FormatContext>
	auto format(const guild_role& role, FormatContext& ctx) {
		return fmt::formatter<std::string_view, Char>::format(role.to_mentionable_string(), ctx);
	}
};

constexpr static int asddjktasduksadasdsuhkasda = sizeof(guild_role);