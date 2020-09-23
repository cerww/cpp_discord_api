#pragma once
#include "User.h"
#include "guild_role.h"
#include <nlohmann/json.hpp>
#include "presence_update.h"
#include "partial_guild_member.h"
#include <range/v3/view/transform.hpp>
#include "../common/higher_order_functions.h"

struct Guild;

struct guild_member :partial_guild_member {
	const Guild& guild() const noexcept;;

	auto roles() const {
		return role_ids() | ranges::views::transform(hof::map_with(id_to_role_map()));
	}

	// auto roles_sorted() const{
	// 	if(!m_roles_are_sorted) {
	// 		//ranges::sort(m_roles.begin(), m_roles.end(),std::less(),hof::flow(id_to_role_map(),&guild_role::position));			
	// 	}
	// 	return roles();
	// }
	
private:
	ref_count_ptr<Guild> m_guild = nullptr;
	//mutable bool m_roles_are_sorted = false;
	
	discord_obj_map<guild_role> id_to_role_map() const noexcept;
	
	friend void from_json(const nlohmann::json& in, guild_member& out);
	friend struct internal_shard;
};

void from_json(const nlohmann::json& in, guild_member& out);

void to_json(nlohmann::json& out, const guild_member& in);

template<typename Char>
struct fmt::formatter<guild_member, Char> :fmt::formatter<std::string_view, Char> {

	template<typename FormatContext>
	auto format(const guild_member& person, FormatContext& ctx) {
		return fmt::formatter<std::string_view, Char>::format(person.to_mentionable_string(), ctx);
	}
};

