#pragma once
#include "User.h"
#include "timestamp.h"
#include "guild_role.h"
#include <range/v3/all.hpp>
#include <span>
#include <boost/container/small_vector.hpp>

struct partial_guild_member :user {
	std::string_view nick() const noexcept;

	std::span<const snowflake> role_ids() const noexcept {
		return std::span(m_roles.data(),m_roles.size());
	}

	timestamp joined_at() const noexcept;
	bool deaf() const noexcept;
	bool mute() const noexcept;

	bool has_role(snowflake role_id) const noexcept;
	bool has_role(const guild_role& role) const noexcept;

private:
	boost::container::small_vector<snowflake, 5> m_roles{};
	std::string m_nick{};
	timestamp m_joined_at{};
	bool m_deaf = false;
	bool m_mute = false;

	friend void from_json(const nlohmann::json&, partial_guild_member&);
	friend struct internal_shard;
};

constexpr int auidghsadjkashdiuas = sizeof(boost::container::small_vector<snowflake,5>);

constexpr int ashsdgasdasd = sizeof(partial_guild_member);

inline void from_json(const nlohmann::json& in, partial_guild_member& out) {
	in["user"].get_to(static_cast<user&>(out));
	out.m_nick = in.value("nick",std::string(""));
	//out.m_roles = in["roles"].get<std::vector<snowflake>>();
	out.m_roles = in["roles"] | ranges::views::transform(&nlohmann::json::get<snowflake>) | ranges::to<boost::container::small_vector<snowflake, 5>>();
	
	//out.m_joined_at = in["joined_at"].get<timestamp>();
	
	out.m_deaf = in["deaf"].get<bool>();
	out.m_mute = in["mute"].get<bool>();
}
