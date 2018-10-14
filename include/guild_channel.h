#pragma once
#include "partial_channel.h"
#include <experimental/generator>
#include "range-like-stuffs.h"

class channel_catagory;

class Guild;

class guild_channel:public partial_channel{
public:
	snowflake guild_id() const noexcept;
	//Guild& guild() noexcept;
	const Guild& guild() const noexcept;
	const std::vector<permission_overwrite>& permission_overwrites() const noexcept;
	bool nsfw() const noexcept;
	int position() const noexcept;
	snowflake catagory_id() const noexcept;
	//channel_catagory& parent() noexcept;
	const channel_catagory& parent() const noexcept;
	bool has_parent() const noexcept;
	const std::vector<permission_overwrite>& parent_overwrites()const noexcept;
	std::experimental::generator<permission_overwrite> total_permissions()const {
		if (m_parent)
			return concat(permission_overwrites(), parent_overwrites());
		return concat(permission_overwrites());
	}
private:
	snowflake m_guild_id;
	bool m_nsfw = false;
	int m_position = 0;
	std::vector<permission_overwrite> m_permission_overwrites;

	snowflake m_parent_id;
	channel_catagory* m_parent = nullptr;

	Guild * m_guild = nullptr;

	friend class shard;
	friend void from_json(const nlohmann::json&, guild_channel& g);
};

void from_json(const nlohmann::json& json, guild_channel& g);
