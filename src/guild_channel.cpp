#include "guild_channel.h"
#include "channel_catagory.h"
#include "range-like-stuffs.h"

snowflake guild_channel::guild_id() const noexcept { return m_guild_id; }

const Guild& guild_channel::guild() const noexcept { return *m_guild; }

bool guild_channel::nsfw() const noexcept { return m_nsfw; }

int guild_channel::position() const noexcept { return m_position; }

snowflake guild_channel::catagory_id() const noexcept { return m_parent_id; }

const channel_catagory& guild_channel::parent() const noexcept { return *m_parent; }

bool guild_channel::has_parent() const noexcept { return m_parent; }

void from_json(const nlohmann::json& json, guild_channel& g) {
	from_json(json, static_cast<partial_channel&>(g));
	g.m_guild_id = json.value("guild_id", snowflake());
	g.m_nsfw = json.value("nsfw", false);
	g.m_position = json["position"].get<int>();
	g.m_permission_overwrites = json["permission_overwrites"].get<std::vector<permission_overwrite>>();
	g.m_parent_id = json.value("parent_id", snowflake());
}

ranges::view::all_t<const std::vector<permission_overwrite>&> guild_channel::parent_overwrites() const noexcept {
	return m_parent->permission_overwrites();
}

//std::experimental::generator<permission_overwrite> guild_channel::total_permissions() const
