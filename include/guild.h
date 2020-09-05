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
#include "../common/optional_ref.h"
#include <range/v3/view/all.hpp>
#include "../common/higher_order_functions.h"
#include "channel_catagory.h"
#include "ref_stable2.h"

struct internal_shard;

struct Guild :partial_guild/*,thread_unsafe_ref_counted_but_different<Guild>*/ {

	Guild() = default;
	Guild(const Guild&) = delete;
	Guild(Guild&&) = delete;

	Guild& operator=(const Guild&) = delete;
	Guild& operator=(Guild&&) = delete;

	~Guild() = default;

	timestamp joined_at() const noexcept;
	bool large() const noexcept;
	bool unavailable() const noexcept;
	int member_count() const noexcept;

	const text_channel& system_channel() const;

	const auto& members() const noexcept {
		return m_members;
	};

	auto members_list() const noexcept {
		return m_members | ranges::views::values;
	};

	const guild_member& owner() const;
	
	//TODO remove const_casts
	auto text_channels_list() const noexcept {
		return m_text_channel_ids | ranges::views::transform(hof::map_with(text_channels()));
		//return m_text_channels | ranges::views::values;
	}

	auto voice_channels_list() const noexcept {
		return m_voice_channel_ids | ranges::views::transform(hof::map_with(voice_channels()));
		//return m_voice_channels | ranges::views::values;
	}

	auto channel_catagories_list() const noexcept {
		return m_channel_catagory_ids | ranges::views::transform(hof::map_with(channel_catagories()));
		//return m_channel_catagories | ranges::views::values;
	}

	discord_obj_map2<text_channel> text_channels()const noexcept {
		return m_text_channels;
	}

	discord_obj_map2<voice_channel> voice_channels()const noexcept {
		return m_voice_channels;
	}

	discord_obj_map2<channel_catagory> channel_catagories()const noexcept {
		return m_channel_catagories;
	}

	std::span<const snowflake> text_channel_ids() const noexcept {
		return m_text_channel_ids;
	};

	std::span<const snowflake> channel_catagories_ids() const noexcept {
		return m_channel_catagory_ids;
	};

	std::span<const snowflake> voice_channel_ids() const noexcept {
		return m_voice_channel_ids;
	};

	optional_ref<const voice_channel> afk_channel() const;

	std::span<const voice_state> voice_states() const noexcept {
		return m_voice_states;
	};

	std::optional<activity> activity_for(snowflake id) const noexcept {
		const auto it = m_activities.find(id);
		if (it == m_activities.end()) {
			return std::nullopt;
		} else {
			return it->second;
		}
	}
	
	std::optional<activity> activity_for(const partial_guild_member& member) const noexcept {
		return activity_for(member.id());
	}

	std::optional<Status> status_for(const snowflake id) const noexcept{
		const auto it = m_status.find(id);
		if (it == m_status.end()) {
			return std::nullopt;
		} else {
			return it->second;
		}
	}

	std::optional<Status> status_for(const partial_guild_member& member) const noexcept {
		return status_for(member.id());
	}

	optional_ref<const voice_channel> voice_channel_for(const partial_guild_member& member)const {
		const auto it = ranges::find(m_voice_states, member.id(), &voice_state::user_id);
		if(it  == m_voice_states.end()) {
			return std::nullopt;
		}else {
			return m_voice_channels.at(it->channel_id());
		}
	}

private:
	//the following is used for conveniance only


	timestamp m_joined_at = {};
	bool m_large = false;
	bool m_unavailable = false;
	int m_member_count = 0;

	ref_stable_map<snowflake, guild_member> m_members{};//keep ref stable?
	ska::bytell_hash_map<snowflake, std::optional<activity>> m_activities;
	ska::bytell_hash_map<snowflake, Status> m_status;
	
	ref_stable_map2<snowflake,text_channel> m_text_channels;
	ref_stable_map2<snowflake,voice_channel> m_voice_channels;
	ref_stable_map2<snowflake,channel_catagory> m_channel_catagories;

	//non-const version used for conveniance in shard.cpp
	//returns mutable members so it has to be private
	auto mutable_members_list() noexcept {
		return m_members | ranges::views::values;
	}

	std::vector<snowflake> m_text_channel_ids{};
	std::vector<snowflake> m_voice_channel_ids{};
	std::vector<snowflake> m_channel_catagory_ids{};
	std::vector<voice_state> m_voice_states{};


	internal_shard* m_shard = nullptr;

	bool m_is_ready = true;
	nlohmann::json m_presences;
	friend void from_json(const nlohmann::json& json, Guild& guild);
	friend struct internal_shard;
};

constexpr int ausdhasdasdasd = sizeof(Guild);

void from_json(const nlohmann::json& json, Guild& guild);
