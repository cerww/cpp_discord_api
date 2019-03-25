#pragma once
#include <range/v3/all.hpp>
#include "Guild.h"
#include <memory_resource>
#include "allocatey.h"
#include <unordered_map>
//this file should always compile
#pragma warning(push,0)
CONCEPT_ASSERT(ranges::ForwardRange<discord_obj_list<guild_member, std::vector<snowflake>&>>());
CONCEPT_ASSERT(ranges::BidirectionalRange<discord_obj_list<guild_member, std::vector<snowflake>&>>());
CONCEPT_ASSERT(ranges::RandomAccessRange<discord_obj_list<guild_member, std::vector<snowflake>&>>());
CONCEPT_ASSERT(ranges::EqualityComparable<discord_obj_list<guild_member, std::vector<snowflake>&>::iterator>());
CONCEPT_ASSERT(ranges::SizedSentinel<discord_obj_list<guild_member, std::vector<snowflake>&>::iterator, discord_obj_list<guild_member, std::vector<snowflake>&>::iterator>());
CONCEPT_ASSERT(ranges::TotallyOrdered<discord_obj_list<guild_member, std::vector<snowflake>&>::iterator>());
CONCEPT_ASSERT(ranges::RandomAccessIterator<discord_obj_list<guild_member, std::vector<snowflake>&>::iterator>());
CONCEPT_ASSERT(ranges::ForwardRange<discord_obj_map<guild_member>>());

inline void test_rand_itness_of_discord_obj_list(discord_obj_list<guild_member,std::vector<snowflake>&>& t) {
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

/*
inline void test_indirect_tgingy() {
	auto lamby = [](int& c) {std::cout << c << std::endl; };
	auto lamby2 = [](int c) {std::cout << c << std::endl; };
	single_chunk_mem_pool e;
	indirect<int, single_chunk_allocator<int>> a(e);
	a = 12;
	indirect<int, single_chunk_allocator<int>> aa = a;

	indirect<int, single_chunk_allocator<int>> aab = std::move(aa);

	std::pmr::monotonic_buffer_resource pool;
	indirect<int, std::pmr::polymorphic_allocator<int>> qwe(&pool, 2);
	lamby(qwe);
	lamby(a);
	lamby2(a);
	double y = a;
	int i = a;
	indirect<double> d = a;
	auto qwert = a == d;
	auto qwea = d == y;

	std::unordered_map<int, indirect<int>> mapu;
	std::unordered_map<int, indirect<std::vector<int>>> mapua;
	mapua[2].value().emplace_back();

	mapu[2] = d;
	mapu[2] = 9;
	mapu.insert(std::make_pair(1, 2));
	mapu[3] = qwe;
	mapu[qwe] = aab;
	mapu.insert(std::make_pair(65, aa));

	std::vector<indirect<int>> vecy;
	vecy.push_back(d);
	vecy.emplace_back(3);
	int& lastu = vecy.emplace_back();
	vecy.erase(vecy.begin());
	std::vector<indirect<std::vector<int>>> rawrland;

	rawrland.emplace_back().value().emplace_back();
	std::vector<int> weutyhf = std::move(rawrland.back());
	rawrland.pop_back();
	rawrland.emplace_back(std::vector<int>{1, 2, 3, 4});

	std::vector<indirect<std::vector<int>>> hjk;
	hjk.push_back({ 1,2,3,4 });
	hjk.emplace_back(std::vector<int>{1, 2, 34});
	//hjk.emplace_back({1, 2, 34});

	lamby2(std::move(d));
	d = a;
	d = a + 3;//std::move(a);
	//a = std::move(a);
	indirect<size_t> lop(a);
	indirect<double> c = a;
	indirect<double> t = c + 2;
	auto aweqasd = c < qwe + qwe > 0 + 0 > a + 1;

	std::vector<int> v;

	v.push_back(a + y + i + d);
	std::cout << d << std::endl;
}

*/
#pragma warning(pop)




