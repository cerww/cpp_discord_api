#pragma once
#include "permision_overwrite.h"
#include "permission.h"
#include "guildMember.h"
#include "guild_channel.h"
#include "range-like-stuffs.h"

inline permission merge_permissions(permission p1, permission p2) {
	return permission(p1.data() | p2.data());
}

template<typename range>
std::enable_if_t<is_range_of_v<range,permission_overwrite>,permission> merge_permissions(permission p ,range&& overwrites) {
	for(auto&& overwrite:overwrites) {
		p.addPermissions(overwrite.allow);
		p.removePermissions(overwrite.deny);
	}return p;
}

inline permission merge_permissions(const guild_member& member) {
	permission retVal;
	for(auto&& role:member.roles()) 
		retVal = merge_permissions(retVal, role.permissions());
	return retVal;
}

template<typename rng>
std::enable_if_t<is_range_of_v<rng,permission_overwrite>,permission> merge_permissions(const guild_member& member, const rng& range) {
	return merge_permissions(merge_permissions(member), filter(range, [&](permission_overwrite p) {
		if (p.type == overwrite_type::member) {
			return member.id() == p.id;
		}return member.has_role(p.id);
	}));
}

inline permission merge_permissions(const guild_member& member, const guild_channel& channel) {
	return merge_permissions(merge_permissions(member), channel.permission_overwrites());
}

