#pragma once
#include "User.h"
#include "timestamp.h"
#include "guild_role.h"
#include <range/v3/view/all.hpp>

struct partial_guild_member :user {
	std::string_view nick() const noexcept;

	auto role_ids() const noexcept {
		return m_roles | ranges::views::all;
	}

	timestamp joined_at() const noexcept;
	bool deaf() const noexcept;
	bool mute() const noexcept;

	bool has_role(snowflake role_id) const noexcept;
	bool has_role(const guild_role& role) const noexcept;

private:
	std::string m_nick{};
	std::vector<snowflake> m_roles{};
	timestamp m_joined_at{};
	bool m_deaf = false;
	bool m_mute = false;

	friend void from_json(const nlohmann::json&, partial_guild_member&);
	friend struct shard;
};


inline void from_json(const nlohmann::json& in, partial_guild_member& out) {
	from_json(in["user"], static_cast<user&>(out));
	const auto it = in.find("nick");
	if (it != in.end())
		out.m_nick = in["nick"].is_null() ? "" : in["nick"].get<std::string>();
	out.m_roles = in["roles"].get<std::vector<snowflake>>();
	//out.m_joined_at = in["joined_at"].get<timestamp>();
	out.m_deaf = in["deaf"].get<bool>();
	out.m_mute = in["mute"].get<bool>();
}
