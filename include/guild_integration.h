#pragma once
#include "snowflake.h"
#include <optional>
#include "User.h"
#include "timestamp.h"

struct integration_account {

	std::string_view id() const noexcept { return m_id; }

	std::string_view name() const noexcept { return m_name; }

private:
	std::string m_id;
	std::string m_name;
	friend void from_json(const nlohmann::json&, integration_account&);
};

inline void from_json(const nlohmann::json& json, integration_account& me) {
	me.m_id = json["id"].get<std::string>();
	me.m_name = json["name"].get<std::string>();
}

struct integration_expire_behavior {


private:

	friend void from_json(const nlohmann::json& json, integration_expire_behavior& me) {
		//wat
	}
};

struct partial_integration {
	snowflake id() const {
		return m_id;
	};

	std::string_view name() const {
		return m_name;
	};

	std::string_view type() const {
		return m_type;
	};


	const integration_account& account() const {
		return m_account;
	};

private:
	snowflake m_id;
	std::string m_name;
	std::string m_type;
	integration_account m_account;

	friend void from_json(const nlohmann::json& json, partial_integration& me) {
		me.m_id = json["id"].get<snowflake>();
		me.m_name = json["name"].get<std::string>();
		me.m_type = json["type"].get<std::string>();
		me.m_account = json["account"].get<integration_account>();
	}
};

struct guild_integration:partial_integration {	

	bool enabled() const {
		return m_enabled;
	};

	bool syncing() const {
		return m_syncing;
	};

	snowflake role_id() const {
		return m_role_id;
	};

	std::optional<bool> enable_emoticons() const {
		return m_enable_emoticons;
	};

	const integration_expire_behavior& expire_behavior() const {
		return m_expire_behavior;
	};

	int expire_grace_period() const {
		return m_expire_grace_period;
	};

	const user& user() const {
		return m_user;
	};


	timestamp synced_at() const noexcept {
		return m_synced_at;
	};


private:
	bool m_enabled = false;
	bool m_syncing = false;
	snowflake m_role_id;
	std::optional<bool> m_enable_emoticons;//wat optional<bool> ???
	integration_expire_behavior m_expire_behavior;
	int m_expire_grace_period = 0;
	::user m_user;
	timestamp m_synced_at;

	friend void from_json(const nlohmann::json& json, guild_integration& me) {
		me.m_enabled = json["enabled"].get<bool>();
		me.m_syncing = json["syncing"].get<bool>();
		me.m_role_id = json["role_id"].get<snowflake>();
		me.m_enable_emoticons = json.value("enable_emoticons", std::optional<bool>());
		me.m_expire_behavior = json["expire_behavior"].get<integration_expire_behavior>();
		me.m_expire_grace_period = json["expire_grace_period"].get<int>();
		me.m_user = json["user"].get<::user>();
		me.m_synced_at = json["synced_at"].get<timestamp>();
	}
};
