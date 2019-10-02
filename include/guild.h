#pragma once
#include "partial_channel.h"
#include "text_channel.h"
#include "guild_member.h"
#include <nlohmann/json.hpp>
#include "snowflake.h"
#include "timestamp.h"
#include "voice_channel.h"
#include "partial_guild.h"
#include "voice_state.h"
#include "optional_ref.h"
#include <range/v3/view/all.hpp>
#include "higher_order_functions.h"
#include "channel_catagory.h"

struct shard;

struct Guild: partial_guild{
	timestamp joined_at() const noexcept;
	bool large() const noexcept;
	bool unavailable() const noexcept;
	int member_count() const noexcept;
	const text_channel& system_channel() const noexcept;
	auto members() const noexcept {
		return m_members | ranges::views::all;
	};
	auto members_list() const noexcept {
		return m_members | ranges::views::values;
	};

	const guild_member& owner()const noexcept;
	
	auto text_channels()const noexcept {
		return m_text_channels | ranges::views::transform(hof::map_with(all_text_channels()));
	}

	auto voice_channels()const noexcept {
		return m_voice_channels | ranges::views::transform(hof::map_with(all_voice_channels()));
	}

	auto channel_catagories()const noexcept{
		return m_channel_catagories | ranges::views::transform(hof::map_with(all_channel_catagories()));
	}

	auto text_channel_ids()const noexcept {
		return m_text_channels | ranges::views::all;
	};

	auto channel_catagories_ids()const noexcept {
		return m_channel_catagories | ranges::views::all;
	};

	auto voice_channel_ids()const noexcept {
		return m_voice_channels | ranges::views::all;
	};

	optional_ref<const voice_channel> afk_channel()const noexcept;
	auto voice_states() const noexcept {
		return m_voice_states | ranges::views::all;
	};
private:
	discord_obj_map<text_channel> all_text_channels()const noexcept;
	discord_obj_map<voice_channel> all_voice_channels()const noexcept;
	discord_obj_map<channel_catagory> all_channel_catagories()const noexcept;


	timestamp m_joined_at = {};
	bool m_large = false;
	bool m_unavailable = false;
	int m_member_count = 0;

	ref_stable_map<snowflake, guild_member> m_members{};

	//non-const version used for conveniance in shard.cpp
	//returns mutable members so it has to be private
	auto mutable_members_list()noexcept {
		return m_members | ranges::views::values;
	}

	std::vector<snowflake> m_text_channels{};	
	std::vector<snowflake> m_voice_channels{};
	std::vector<snowflake> m_channel_catagories{};
	std::vector<voice_state> m_voice_states{};


	shard* m_shard = nullptr;

	bool m_is_ready = true;
	nlohmann::json m_presences;
	friend void from_json(const nlohmann::json& json, Guild& guild);
	friend struct shard;
};

void from_json(const nlohmann::json& json, Guild& guild);





