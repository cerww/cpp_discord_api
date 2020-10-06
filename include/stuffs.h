#pragma once
#include "permision_overwrite.h"
#include "permission.h"
#include "guild_member.h"
#include "guild_channel.h"
#include "../common/range-like-stuffs.h"
#include <range/v3/view/filter.hpp>
#include "../common/higher_order_functions.h"
#include <variant>
#include "guild.h"

inline permission combined_permissions(const permission& p1, const permission& p2) {
	return p1 | p2;
}

template<typename range>
std::enable_if_t<is_range_of_v<range, permission_overwrite>, permission> combined_permissions(permission return_val, range&& overwrites) {
	for (auto&& overwrite : overwrites) {
		return_val.add_permissions(overwrite.allow());
		return_val.remove_permissions(overwrite.deny());
	}
	return return_val;
}

template<typename roles_range, typename overwrite_range_t>//TODO replace with concepts
std::enable_if_t<
	is_range_of_v<overwrite_range_t, permission_overwrite>
	&& is_range_of_v<roles_range, guild_role>, permission> combined_permissions(roles_range&& roles , overwrite_range_t&& overwrites, const Guild& guild) {
	
	std::vector<std::variant<const guild_role*, const permission_overwrite*>> perms_list_all;

	if constexpr (ranges::sized_range<roles_range> && ranges::sized_range<overwrite_range_t>) {
		perms_list_all.reserve(overwrites.size() + roles.size()+1);
	} else if constexpr (ranges::sized_range<overwrite_range_t>) {
		perms_list_all.reserve(overwrites.size() + 1 + 3);
	} else if constexpr (ranges::sized_range<roles_range>) {
		perms_list_all.reserve(roles.size()+1);
	} else {
		perms_list_all.reserve(2 + 1);//random number, should always contain at least 1 since @everyone role
	}
	
	perms_list_all.push_back(&guild.roles()[guild.id()]);
	ranges::actions::push_back(perms_list_all, roles | ranges::views::addressof);
	ranges::actions::push_back(perms_list_all, overwrites | ranges::views::addressof);
	
	std::sort(perms_list_all.begin(), perms_list_all.end(), [&](const auto& a,const auto& b) {
		if (std::holds_alternative<permission_overwrite>(b) 
			&& std::get<permission_overwrite>(b).type() == overwrite_type::member)
		{			
			return true;
		}else if (std::holds_alternative<permission_overwrite>(a) 
			&& std::get<permission_overwrite>(a).type() == overwrite_type::member) 
		{
			return false;
		}

		const int idx_a = [&]() {
			if(std::holds_alternative<const guild_role*>(a)) {
				return std::get<const guild_role*>(a)->position();
			}else {
				return guild.roles()[std::get<const permission_overwrite*>(a)->id()].position();
			}
		}();

		const int idx_b = [&]() {
			if (std::holds_alternative<const guild_role*>(b)) {
				return std::get<const guild_role*>(b)->position();
			} else {
				return guild.roles()[std::get<const permission_overwrite*>(a)->id()].position();
			}
		}();
		
		return idx_a < idx_b;
		
	});
	
	permission ret_val;	
	for (const auto& overwrite_or_role: perms_list_all) {
		if(std::holds_alternative<const guild_role*>(overwrite_or_role)) {
			const auto& role = *std::get<const guild_role*>(overwrite_or_role);
			ret_val.add_permissions(role.permissions());
		}else {
			const auto overwrite = std::get<permission_overwrite>(overwrite_or_role);
			ret_val.add_permissions(overwrite.allow());
			ret_val.remove_permissions(overwrite.deny());
		}
	}
	return ret_val;
}

inline permission combined_permissions(const guild_member& member) {
	//permission retVal;
	//for (auto&& role : member.roles()) {
	//	retVal = combined_permissions(retVal, role.permissions());
	//}
	//return retVal;
	auto perms = member.roles() | ranges::views::transform(&guild_role::permissions);
	return std::accumulate(perms.begin(), perms.end(), permission());
}

template<typename rng>
std::enable_if_t<is_range_of_v<rng, permission_overwrite>, permission> combined_permissions(const guild_member& member, rng&& range) {
	return combined_permissions(member.roles(), range | ranges::views::filter([&](const permission_overwrite& p) {
		if (p.type() == overwrite_type::member) {
			return member.id() == p.id();
		}
		return member.has_role(p.id());
	}),member.guild());
	/*
	auto fn = 
		hof::logical_disjunction(
			hof::logical_conjunction(hof::flow(&permission_overwrite::type,hof::is_equal_to(overwrite_type::member)),
									 hof::flow(&permission_overwrite::id,hof::is_equal_to(member.id()))),
			hof::flow(&permission_overwrite::id,hof::bind1st(&guild_member::has_role,member))
		);
		(&permission_overwrite::type |> hof::is_equal_to(overwrite_type::member)
		&& &permission_overwrite::id |> hof::is_equal_to(member.id()))
		|| &permission_overwrite::id |> member.has_role
	*/
}


inline permission combined_permissions(const guild_member& member, const guild_channel& channel) {
	return combined_permissions(member, channel.permission_overwrites());
}

inline permission permissions_in_channel(const guild_member& member, const guild_channel& channel) {
	return combined_permissions(member, channel);
}
