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
#include "../common/ref_stable2.h"
#include "../common/thingy_that_prints_after_destroyed.h"

struct internal_shard;

struct Guild :ref_counted, partial_guild {

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

	discord_obj_map<guild_member> members() const {
		return m_members;
	}

	auto members_list() const {
		throw_if_dead();
		return m_members | ranges::views::values;
	}

	const guild_member& owner() const;

	auto text_channels_list() const {
		throw_if_dead();
		return m_text_channel_ids | ranges::views::transform(hof::map_with(text_channels()));		
	}

	auto voice_channels_list() const {
		throw_if_dead();
		return m_voice_channel_ids | ranges::views::transform(hof::map_with(voice_channels()));		
	}

	auto channel_catagories_list() const {
		throw_if_dead();
		return m_channel_catagory_ids | ranges::views::transform(hof::map_with(channel_catagories()));		
	}

	discord_obj_map2<text_channel> text_channels() const {
		throw_if_dead();
		return m_text_channels;
	}

	discord_obj_map2<voice_channel> voice_channels() const  {
		throw_if_dead();
		return m_voice_channels;
	}

	discord_obj_map2<channel_catagory> channel_catagories() const  {
		throw_if_dead();
		return m_channel_catagories;
	}

	std::span<const snowflake> text_channel_ids() const noexcept {		
		return m_text_channel_ids;
	}

	std::span<const snowflake> channel_catagories_ids() const noexcept {
		return m_channel_catagory_ids;
	}

	std::span<const snowflake> voice_channel_ids() const noexcept {
		return m_voice_channel_ids;
	}

	optional_ref<const voice_channel> afk_channel() const {
		throw_if_dead();
		if (afk_channel_id().val) {
			return m_voice_channels.at(afk_channel_id());
		}
		return std::nullopt;
	}

	std::span<const voice_state> voice_states() const noexcept {
		return m_voice_states;
	}

	std::optional<std::span<const activity>> activity_for(snowflake id) const noexcept {
		const auto it = m_activities.find(id);
		if (it == m_activities.end()) {
			return std::nullopt;
		} else {
			return it->second;
		}
	}

	std::optional<std::span<const activity>> activity_for(const partial_guild_member& member) const noexcept {
		return activity_for(member.id());
	}

	std::optional<Status> status_for(const snowflake id) const noexcept {
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

	optional_ref<const voice_channel> voice_channel_for(const partial_guild_member& member) const {
		throw_if_dead();
		const auto it = ranges::find(m_voice_states, member.id(), &voice_state::user_id);
		if (it == m_voice_states.end()) {
			return std::nullopt;
		} else {
			return m_voice_channels.at(it->channel_id());
		}
	}

	const guild_member& my_member() const{
		throw_if_dead();
		return m_members.at(m_bot_id);	
	}
	
private:
	timestamp m_joined_at = {};
	bool m_large = false;
	bool m_unavailable = false;
	int m_member_count = 0;

	ref_stable_map<snowflake, guild_member> m_members{};//keep ref stable?
	ska::bytell_hash_map<snowflake, std::vector<activity>> m_activities;
	ska::bytell_hash_map<snowflake, Status> m_status;

	ref_stable_map2<snowflake, text_channel> m_text_channels;
	ref_stable_map2<snowflake, voice_channel> m_voice_channels;
	ref_stable_map2<snowflake, channel_catagory> m_channel_catagories;

	//non-const version used for conveniance in internal_shard.cpp
	//returns mutable members so it has to be private
	auto mutable_members_list() noexcept {
		return m_members | ranges::views::values;
	}

	std::vector<snowflake> m_text_channel_ids{};
	std::vector<snowflake> m_voice_channel_ids{};
	std::vector<snowflake> m_channel_catagory_ids{};
	std::vector<voice_state> m_voice_states{};

	//thingy_that_prints_after_destroyed m_aaaaaa = thingy_that_prints_after_destroyed("guild_dead");

	internal_shard* m_shard = nullptr;

	bool m_is_ready = true;
	nlohmann::json m_presences;
	bool m_is_dead = false;

	snowflake m_bot_id;
	
	void throw_if_dead()const  {
		if(m_is_dead) {
			throw std::runtime_error("guild is dead");
		}
	}
	
	void set_dead() {
		m_text_channels.clear();
		m_channel_catagories.clear();
		m_voice_channels.clear();
		m_members.clear();
		
		m_is_dead = true;
	}
	
	friend void from_json(const nlohmann::json& json, Guild& guild);
	friend struct internal_shard;
	friend struct guild_lifetime_extender;
};

constexpr int ausdhasdasdasdjkghkjg = sizeof(Guild);

void from_json(const nlohmann::json& json, Guild& guild);

struct guild_lifetime_extender {
	guild_lifetime_extender() = default;
	
	guild_lifetime_extender(Guild& g) :m_guild(&g){
		
	}
	
	guild_lifetime_extender(const guild_lifetime_extender&) = default;
	guild_lifetime_extender(guild_lifetime_extender&&) = default;
	guild_lifetime_extender& operator=(const guild_lifetime_extender&) = default;
	guild_lifetime_extender& operator=(guild_lifetime_extender&&) = default;
	
	~guild_lifetime_extender() {
		m_guild->set_dead();
	}

	const Guild& guild()const noexcept {
		return *m_guild;
	}

	bool operator==(const guild_lifetime_extender&) const noexcept = default;
	
private:
	ref_count_ptr<Guild> m_guild;	
};
