#pragma once
#include <range/v3/all.hpp>
#include "Guild.h"


CONCEPT_ASSERT(ranges::ForwardRange<discord_obj_list<guild_member, std::vector<snowflake>&>>());
CONCEPT_ASSERT(ranges::BidirectionalRange<discord_obj_list<guild_member, std::vector<snowflake>&>>());
CONCEPT_ASSERT(ranges::RandomAccessRange<discord_obj_list<guild_member, std::vector<snowflake>&>>());
CONCEPT_ASSERT(ranges::EqualityComparable<discord_obj_list<guild_member, std::vector<snowflake>&>::iterator>());
CONCEPT_ASSERT(ranges::SizedSentinel<discord_obj_list<guild_member, std::vector<snowflake>&>::iterator, discord_obj_list<guild_member, std::vector<snowflake>&>::iterator>());
CONCEPT_ASSERT(ranges::TotallyOrdered<discord_obj_list<guild_member, std::vector<snowflake>&>::iterator>());
CONCEPT_ASSERT(ranges::RandomAccessIterator<discord_obj_list<guild_member, std::vector<snowflake>&>::iterator>());
CONCEPT_ASSERT(ranges::ForwardRange<discord_obj_map<guild_member>>());

inline void wqeftyuhgdskjx(discord_obj_list<guild_member,std::vector<snowflake>&>& t) {
	auto it = t.begin();
	++it;
	it++;
	it--;
	it += 1;
	it - 1;
	it -= 1;
	it[2];
	static_assert(std::is_same_v<decltype(it)::iterator_category, std::random_access_iterator_tag>);
	it < it;
	it >= it;
	it[it - it];
	it + (it - it);
	it == it;
	it != it;
	it->guild();
	
}