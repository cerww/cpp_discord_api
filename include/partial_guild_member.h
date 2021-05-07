#pragma once
#include "User.h"
#include "timestamp.h"
#include "guild_role.h"
#include <range/v3/all.hpp>
#include <span>
//#include <boost/container/small_vector.hpp>
#include "../common/sbo_vector.h"
#include <folly/FBString.h>

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

protected:
	//boost::container::small_vector<snowflake, 5> m_roles{};
	sbo_vector<snowflake, 5> m_roles;
	folly::fbstring m_nick{};	
	timestamp m_joined_at{};
	bool m_deaf = false;
	bool m_mute = false;

	friend void from_json(const nlohmann::json&, partial_guild_member&);
	friend struct internal_shard;
};

//constexpr int auidghsadjkashdiuas = sizeof(boost::container::small_vector<snowflake,5>);

constexpr int ashsdgasdasd = sizeof(partial_guild_member);

inline void from_json(const nlohmann::json& in, partial_guild_member& member) {
	in["user"].get_to(static_cast<user&>(member));
	if (in.contains("nick") && !in["nick"].is_null()) {
		member.m_nick = in.value("nick", std::string(""));
	}
	
	//out.m_roles = in["roles"].get<std::vector<snowflake>>();
	
	member.m_roles.reserve(in["roles"].size());
	ranges::push_back(member.m_roles, in["roles"] | ranges::views::transform(&nlohmann::json::get<snowflake>));
	
	//out.m_joined_at = in["joined_at"].get<timestamp>();
	
	member.m_deaf = in["deaf"].get<bool>();
	member.m_mute = in["mute"].get<bool>();
}

template<typename Char>
struct fmt::formatter<partial_guild_member, Char> :fmt::formatter<std::string_view, Char> {

	template<typename FormatContext>
	auto format(const partial_guild_member& person, FormatContext& ctx) {
		return fmt::formatter<std::string_view, Char>::format(person.to_mentionable_string(), ctx);
	}
};

