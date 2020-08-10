#pragma once
#include "snowflake.h"
#include "User.h"
#include "../common/common/optional_from_json.h"
#include <optional>

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
		if (!json["avatar"].is_null()) {
			hook.m_avatar = json["avatar"].get<std::string>();
		}
		//hook.m_avatar = json.value("avatar", std::optional<std::string>());
		hook.m_token = json.value("token", std::optional<std::string>());
	}
};


namespace webhook_settings {
	struct name {
		std::string n;
		static constexpr const char* vname = "name";
	};


	struct avatar {
		std::string n;
		static constexpr const char* vname = "avatar";
	};

	struct channel_id {
		snowflake n;
		static constexpr const char* vname = "channel_id";
	};
}


template<typename... setting>
struct modify_webhook_settings {

	auto name(std::string n) {
		return modify_webhook_settings<setting..., webhook_settings::name>{
			std::tuple_cat(	std::move(settings), 
							std::tuple(webhook_settings::name{std::move(n)}))
		};
	}
	
	//TODO change type
	auto avatar(std::string n) {
		return modify_webhook_settings<setting..., webhook_settings::avatar>{
			std::tuple_cat(	std::move(settings), 
							std::tuple(webhook_settings::avatar{std::move(n)}
							)
			)
		};
	}

	auto channel_id(snowflake n) {
		return modify_webhook_settings<setting..., webhook_settings::channel_id>{
			std::tuple_cat(	std::move(settings), 
							std::tuple(webhook_settings::channel_id{ n }))
		};
	}

	std::tuple<setting...> settings;
};
