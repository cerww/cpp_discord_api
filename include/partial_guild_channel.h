#pragma once
#include "snowflake.h"
#include "partial_channel.h"

struct partial_guild_channel :partial_channel {
	snowflake guild_id() const noexcept { return m_guild_id; }

	bool nsfw() const noexcept { return m_nsfw; }

	int position() const noexcept { return m_position; }

	snowflake catagory_id() const noexcept { return m_parent_id; }
	
	std::span<const permission_overwrite> permission_overwrites() const noexcept {
		return m_permission_overwrites;
	};
	
private:
	snowflake m_guild_id;
	bool m_nsfw = false;
	int m_position = 0;
	std::vector<permission_overwrite> m_permission_overwrites;

	snowflake m_parent_id;
	friend void from_json(const nlohmann::json& json, partial_guild_channel& g);
	friend struct internal_shard;
};


inline void from_json(const nlohmann::json& json,partial_guild_channel& g) {
	from_json(json, static_cast<partial_channel&>(g));
	g.m_guild_id = json.value("guild_id", snowflake());
	g.m_nsfw = json.value("nsfw", false);
	g.m_position = json["position"].get<int>();
	g.m_permission_overwrites = json["permission_overwrites"].get<std::vector<permission_overwrite>>();
	g.m_parent_id = json.value("parent_id", snowflake());
}
