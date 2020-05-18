#include "guild_channel.h"
#include "channel_catagory.h"
#include "range-like-stuffs.h"

const Guild& guild_channel::guild() const noexcept { return *m_guild; }


const channel_catagory& guild_channel::parent() const noexcept { return *m_parent; }

bool guild_channel::has_parent() const noexcept { return m_parent; }

void from_json(const nlohmann::json& json, guild_channel& g) {
	from_json(json, static_cast<partial_guild_channel&>(g));
	
}

ranges::views::all_t<const std::vector<permission_overwrite>&> guild_channel::parent_overwrites() const noexcept {
	return m_parent->permission_overwrites();
}

//std::experimental::generator<permission_overwrite> guild_channel::total_permissions() const
