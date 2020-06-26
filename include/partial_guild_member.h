#pragma once
#include "User.h"
#include "timestamp.h"
#include "guild_role.h"
<<<<<<< HEAD
#include <range/v3/all.hpp>
#include <span>
#include <boost/container/small_vector.hpp>
=======
#include <range/v3/view/all.hpp>
#include <span>
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de

struct partial_guild_member :user {
	std::string_view nick() const noexcept;

	std::span<const snowflake> role_ids() const noexcept {
<<<<<<< HEAD
		return std::span(m_roles.data(),m_roles.size());
=======
		return m_roles;
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
	}

	timestamp joined_at() const noexcept;
	bool deaf() const noexcept;
	bool mute() const noexcept;

	bool has_role(snowflake role_id) const noexcept;
	bool has_role(const guild_role& role) const noexcept;

private:
	std::string m_nick{};
	boost::container::small_vector<snowflake, 5> m_roles{};
	timestamp m_joined_at{};
	bool m_deaf = false;
	bool m_mute = false;

	friend void from_json(const nlohmann::json&, partial_guild_member&);
	friend struct internal_shard;
};

//constexpr int auidghsa djkashdiuas = sizeof(boost::container::small_vector<snowflake,5>);


inline void from_json(const nlohmann::json& in, partial_guild_member& out) {
	from_json(in["user"], static_cast<user&>(out));
	const auto it = in.find("nick");
	if (it != in.end())
		out.m_nick = in["nick"].is_null() ? "" : in["nick"].get<std::string>();
	//out.m_roles = in["roles"].get<std::vector<snowflake>>();
	out.m_roles = in["roles"] | ranges::views::transform(&nlohmann::json::get<snowflake>) | ranges::to<boost::container::small_vector<snowflake, 5>>();
	
	//out.m_joined_at = in["joined_at"].get<timestamp>();
	
	out.m_deaf = in["deaf"].get<bool>();
	out.m_mute = in["mute"].get<bool>();
}
