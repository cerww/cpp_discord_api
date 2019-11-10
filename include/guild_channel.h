#pragma once
#include "partial_channel.h"
#include "range-like-stuffs.h"
#include <range/v3/all.hpp>

struct channel_catagory;

struct Guild;

struct guild_channel :partial_channel {
	snowflake guild_id() const noexcept;
	const Guild& guild() const noexcept;

	auto permission_overwrites() const noexcept {
		return m_permission_overwrites | ranges::views::all;
	};

	bool nsfw() const noexcept;
	int position() const noexcept;
	snowflake catagory_id() const noexcept;
	ranges::views::all_t<const std::vector<permission_overwrite>&> parent_overwrites() const noexcept;
	const channel_catagory& parent() const noexcept;
	bool has_parent() const noexcept;

	//std::experimental::generator<permission_overwrite> total_permissions()const {
	//
	//fix this somehow
	///*
	ranges::any_view<permission_overwrite> all_permissions() const {
		if (m_parent) {
			return ranges::views::concat(permission_overwrites(), parent_overwrites());
		}
		return permission_overwrites();
	}
	//*/
private:
	snowflake m_guild_id;
	bool m_nsfw = false;
	int m_position = 0;
	std::vector<permission_overwrite> m_permission_overwrites;

	snowflake m_parent_id;
	channel_catagory* m_parent = nullptr;

	Guild* m_guild = nullptr;

	friend struct shard;
	friend void from_json(const nlohmann::json&, guild_channel& g);
};

void from_json(const nlohmann::json& json, guild_channel& g);
