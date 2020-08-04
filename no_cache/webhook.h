#pragma once
#include "snowflake.h"
#include "User.h"
#include <optional>


namespace cacheless {
enum class webhook_type {
	incoming = 1,
	channel_follower = 2
};


struct webhook {
	snowflake id;
	webhook_type type = {};
	snowflake guild_id;
	snowflake channel_id;
	//std::optional<::user> m_user;
	std::optional<std::string> name;
	std::optional<std::string> avatar;
	std::optional<std::string> token;

	friend void from_json(const nlohmann::json& json, webhook& hook) {
		hook.id = json["id"].get<snowflake>();
		hook.type = (webhook_type)json["type"].get<int>();
		hook.guild_id = json.value("guild_id", snowflake());
		hook.channel_id = json["channel_id"].get<snowflake>();
		//hook.m_user = json.value("user", std::optional<::user>());
		hook.name = json.value("name", std::optional<std::string>());
		if (!json["avatar"].is_null()) {
			hook.avatar = json["avatar"].get<std::string>();
		}
		//hook.m_avatar = json.value("avatar", std::optional<std::string>());
		hook.token = json.value("token", std::optional<std::string>());
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
			std::tuple_cat(std::move(settings),
						   std::tuple(webhook_settings::name{std::move(n)}))
		};
	}

	//TODO change type?
	auto avatar(std::string n) {
		return modify_webhook_settings<setting..., webhook_settings::avatar>{
			std::tuple_cat(std::move(settings),
						   std::tuple(webhook_settings::avatar{std::move(n)}
						   )
			)
		};
	}

	auto channel_id(snowflake n) {
		return modify_webhook_settings<setting..., webhook_settings::channel_id>{
			std::tuple_cat(std::move(settings),
						   std::tuple(webhook_settings::channel_id{n}))
		};
	}

	std::tuple<setting...> settings;
};
};
