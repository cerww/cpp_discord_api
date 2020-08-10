#pragma once
#include <nlohmann/json.hpp>
#include "User.h"
#include "timestamp.h"
#include <range/v3/all.hpp>
#include <span>
#include <boost/container/small_vector.hpp>

namespace cacheless {

struct guild_member :user {
	std::vector<snowflake> roles{};
	std::string nick{};
	timestamp joined_at{};
	bool deaf = false;
	bool mute = false;

	friend struct internal_shard;
};

constexpr int auidghsadjkashdiuas = sizeof(boost::container::small_vector<snowflake, 5>);

constexpr int ashsdgasdasd = sizeof(guild_member);

inline void from_json(const nlohmann::json& in, guild_member& out) {
	in["user"].get_to(static_cast<user&>(out));
	if (in.contains("nick") && !in["nick"].is_null()) {
		out.nick = in.value("nick", std::string(""));
	}

	out.roles = in["roles"].get<std::vector<snowflake>>();


	out.deaf = in["deaf"].get<bool>();
	out.mute = in["mute"].get<bool>();
}


}
