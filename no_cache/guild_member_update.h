#pragma once
#include "User.h"
#include "guild_role.h"
#include "timestamp.h"


namespace cacheless {

struct guild_member_update {
	user user;
	std::vector<guild_role> roles;
	std::optional<std::string> nick;
	snowflake guild_id;
	std::optional<timestamp> premium_since;
};

}
