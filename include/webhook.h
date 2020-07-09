#pragma once
#include "snowflake.h"
#include <boost/asio.hpp>
#include "User.h"
#include <optional>
#include <variant>
#include "ref_or_inplace.h"

enum class webhook_type {	
	incoming = 1,
	channel_follower = 2
};


struct webhook {
	snowflake id() const noexcept { return m_id; }

	webhook_type type() const noexcept { return m_type; }

	std::optional<snowflake> guild_id() const noexcept {
		return m_guild_id.val ? m_guild_id : std::optional<snowflake>();
	}

	snowflake channel_id() const noexcept {
		return m_channel_id;
	}

	//can i just ignore this?
	//const std::optional<user>& user()const noexcept { return m_user; }

	std::optional<std::string_view> name() const noexcept { return m_name; }

	std::optional<std::string_view> avatar() const noexcept { return m_avatar; }

	std::optional<std::string_view> token() const noexcept { return m_token; }

private:
	snowflake m_id;
	webhook_type m_type = {};
	snowflake m_guild_id;
	snowflake m_channel_id;
	//std::optional<::user> m_user;
	std::optional<std::string> m_name;
	std::optional<std::string> m_avatar;
	std::optional<std::string> m_token;

	friend void from_json(const nlohmann::json& json, webhook& hook) {
		hook.m_id = json["id"].get<snowflake>();
		hook.m_type = (webhook_type)json["type"].get<int>();
		hook.m_guild_id = json.value("guild_id", snowflake());
		hook.m_channel_id = json["channel_id"].get<snowflake>();
		//hook.m_user = json.value("user", std::optional<::user>());
		hook.m_name = json.value("name", std::optional<std::string>());
		if(!json["avatar"].is_null()) {
			hook.m_avatar = json["avatar"].get<std::string>();
		}
		//hook.m_avatar = json.value("avatar", std::optional<std::string>());
		hook.m_token = json.value("token", std::optional<std::string>());
	}
};



