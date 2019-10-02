#pragma once
#include "permision_overwrite.h"
#include "permission.h"
#include "guild_member.h"
#include "guild_channel.h"
#include "range-like-stuffs.h"
#include <range/v3/view/filter.hpp>
#include "higher_order_functions.h"

constexpr permission combined_permissions(permission p1, permission p2) {
	return p1 | p2;
}

template<typename range>
std::enable_if_t<is_range_of_v<range,permission_overwrite>,permission> combined_permissions(permission return_val,range&& overwrites) {
	for(auto&& overwrite:overwrites) {
		return_val.add_permissions(overwrite.allow());
		return_val.remove_permissions(overwrite.deny());
	}
	return return_val;
}

inline permission combined_permissions(const guild_member& member) {
	permission retVal;
	for(auto&& role:member.roles()) 
		retVal = combined_permissions(retVal, role.permissions());
	return retVal;
}

template<typename rng>
std::enable_if_t<is_range_of_v<rng,permission_overwrite>,permission> combined_permissions(const guild_member& member, rng&& range) {
	return combined_permissions(combined_permissions(member),range |  ranges::views::filter([&](permission_overwrite p) {
		if (p.type() == overwrite_type::member) {
			return member.id() == p.id();
		}
		return member.has_role(p.id());
	}));
	/*
	auto fn = 
		hof::logical_disjunction(
			hof::logical_conjunction(hof::fold(&permission_overwrite::type,hof::is_equal_to(overwrite_type::member)),
									 hof::fold(&permission_overwrite::id,hof::is_equal_to(member.id()))),
			hof::fold(&permission_overwrite::id,hof::bind1st(&guild_member::has_role,member))
		);
	*/
}

inline permission combined_permissions(const guild_member& member, const guild_channel& channel) {
	return combined_permissions(combined_permissions(member), channel.permission_overwrites());
}

