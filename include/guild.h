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

struct internal_shard;

struct Guild :partial_guild {

	Guild() = default;
	Guild(const Guild&) = delete;
	Guild(Guild&&) = default;

	Guild& operator=(const Guild&) = delete;
	Guild& operator=(Guild&&) = default;

	timestamp joined_at() const noexcept;
	bool large() const noexcept;
	bool unavailable() const noexcept;
	int member_count() const noexcept;

	const text_channel& system_channel() const noexcept;

	const auto& members() const noexcept {
		return m_members;
	};

	auto members_list() const noexcept {
		return m_members | ranges::views::values;
	};

	const guild_member& owner() const noexcept;

	auto text_channels() const noexcept {
		return m_text_channel_ids | ranges::views::transform(hof::map_with(all_text_channels()));
	}

	auto voice_channels() const noexcept {
		return m_voice_channel_ids | ranges::views::transform(hof::map_with(all_voice_channels()));
	}

	auto channel_catagories() const noexcept {
		return m_channel_catagory_ids | ranges::views::transform(hof::map_with(all_channel_catagories()));
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

	optional_ref<const voice_channel> afk_channel() const noexcept;

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

	std::optional<Status> status_for(snowflake id) const noexcept{
		const auto it = m_status.find(id);
		if (it == m_status.end()) {
			return std::nullopt;
		}
		else {
			return it->second;
		}
	}

	std::optional<Status> status_for(const partial_guild_member& member) const noexcept {
		return status_for(member.id());
	}

private:
	//following is used for conveniance only
	discord_obj_map<text_channel> all_text_channels() const noexcept;
	discord_obj_map<voice_channel> all_voice_channels() const noexcept;
	discord_obj_map<channel_catagory> all_channel_catagories() const noexcept;


	timestamp m_joined_at = {};
	bool m_large = false;
	bool m_unavailable = false;
	int m_member_count = 0;

	ref_stable_map<snowflake, guild_member> m_members{};
	ref_stable_map<snowflake, std::optional<activity>> m_activities;//no optional<activity&> ;-;
	ska::bytell_hash_map<snowflake, Status> m_status;

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

void from_json(const nlohmann::json& json, Guild& guild);
