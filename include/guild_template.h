#pragma once
#include "snowflake.h"
#include <optional>
#include "User.h"
#include "timestamp.h"
#include "../common/common/optional_from_json.h"

struct guild_template {
	std::string_view code()const noexcept {
		return m_code;
	}

	std::string_view name()const noexcept {
		return m_name;
	}

	std::optional<std::string_view> discription()const noexcept {
		return m_discription;
	}

	int usage_count()const noexcept {
		return m_usage_count;
	}

	snowflake creator_id()const noexcept {
		return m_creator_id;
	}

	user creator()const noexcept {
		return m_creator;
	}

	timestamp created_at()const noexcept {
		return m_created_at;
	}

	timestamp updated_at()const noexcept {
		return m_updated_at;
	}

	snowflake source_guild_id()const noexcept {
		return m_source_guild_id;
	}

	std::optional<bool> is_dirty()const noexcept {
		return m_is_dirty;
	}

private:
	std::string m_code;
	std::string m_name;
	std::optional<std::string> m_discription;
	int m_usage_count = 0;
	snowflake m_creator_id;
	user m_creator;
	timestamp m_created_at;
	timestamp m_updated_at;
	snowflake m_source_guild_id;
	//partial_guild m_serialized_source_guild; //????
	std::optional<bool> m_is_dirty;
	
	friend void from_json(const nlohmann::json& json, guild_template& me) {
		me.m_code = json["code"].get<std::string>();
		me.m_name = json["name"].get<std::string>();
		me.m_discription = json.value("discription", std::optional<std::string>());
		me.m_usage_count = json["usage_count"].get<int>();
		me.m_creator_id = json["creator_id"].get<snowflake>();
		me.m_creator = json["creator"].get<user>();
		me.m_created_at = json["created_at"].get<timestamp>();
		me.m_updated_at= json["updated_at"].get<timestamp>();
		me.m_source_guild_id = json["source_guild_id"].get<snowflake>();
		me.m_is_dirty = json.value("is_dirty", std::optional<bool>());
	}
};


