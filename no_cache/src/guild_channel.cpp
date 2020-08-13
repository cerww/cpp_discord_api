#include "guild_channel.h"
#include "channel_catagory.h"
#include "../common/range-like-stuffs.h"

namespace cacheless {


void from_json(const nlohmann::json& json, guild_channel& g) {
	from_json(json, static_cast<partial_guild_channel&>(g));
}

}
//std::experimental::generator<permission_overwrite> guild_channel::total_permissions() const
