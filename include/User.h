#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include "discord_enums.h"
#include "snowflake.h"
#include <optional>

struct user {
	snowflake id() const noexcept;
	bool is_bot() const noexcept;

	std::string_view username() const noexcept;

	std::optional<int> discriminator() const noexcept;

	std::optional<std::string_view> avatar() const noexcept {
		if (m_avatar.empty()) {
			return std::nullopt;
		} else {
			return m_avatar;
		}
	}

	std::string to_mentionable_string() const {
		return "<@" + std::to_string(m_id.val) + ">";
	}

	int flags() const noexcept {
		return m_flags;
	}

	int public_flags() const noexcept {
		return m_public_flags;
	}

	int premium_type() const noexcept {
		return m_premium_type;
	}

private:
	snowflake m_id;
	std::string m_username;
	std::string m_avatar;
	int m_flags = 0;
	int m_public_flags = 0;
	int16_t m_discriminator = 0;
	int8_t m_premium_type = 0;
	bool m_bot = false;
	bool m_system = false;
	/*
	system?			boolean	
	mfa_enabled?	boolean	
	locale?			string	
	verified?		boolean	
	email?			?string	
	flags?			integer	
	premium_type?	int, 0,1,2
	public_flags?	int
	*/


	friend struct internal_shard;
	friend void from_json(const nlohmann::json&, user& other);
};

constexpr int aasdasdasd = sizeof(user);

void to_json(nlohmann::json& json, const user& other);

void from_json(const nlohmann::json& json, user& other);

template<typename Char>
struct fmt::formatter<user, Char> :fmt::formatter<std::string_view, Char> {
	template<typename FormatContext>
	auto format(const user& person, FormatContext& ctx) {
		return fmt::formatter<std::string_view, Char>::format(person.to_mentionable_string(), ctx);
	}
};
