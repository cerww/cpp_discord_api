#pragma once
#include "partial_channel.h"
#include "range-like-stuffs.h"
#include "partial_guild_channel.h"
#include <range/v3/all.hpp>
#include <span>

struct channel_catagory;

struct Guild;

struct guild_channel :partial_guild_channel {

	const Guild& guild() const noexcept;

	std::span<const permission_overwrite> parent_overwrites() const noexcept;

	const channel_catagory& parent() const noexcept;
	bool has_parent() const noexcept;

	ranges::any_view<permission_overwrite> all_permissions() const {
		if (m_parent) {
			return ranges::views::concat(permission_overwrites(), this->parent_overwrites());
		}
		return permission_overwrites();
	}

private:

	channel_catagory* m_parent = nullptr;

	Guild* m_guild = nullptr;

	friend struct internal_shard;
	friend void from_json(const nlohmann::json&, guild_channel& g);
};

void from_json(const nlohmann::json& json, guild_channel& g);
