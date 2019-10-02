#pragma once
#include <vector>
#include "user.h"
#include "guild_role.h"
#include <nlohmann/json.hpp>
#include "timestamp.h"
#include <optional>
#include "presence_update.h"
#include "partial_guild_member.h"
#include <range/v3/view/transform.hpp>
#include "higher_order_functions.h"

struct Guild;

struct guild_member:partial_guild_member{
	const Guild& guild() const noexcept;;
	auto roles()const {
		return role_ids() | ranges::views::transform(hof::map_with(parent_roles()));
	}
	Status status() const noexcept;
private:
	void set_presence(const partial_presence_update& presence_update);

	discord_obj_map<guild_role> parent_roles()const noexcept;

	Status m_status = Status::online;
	std::optional<activity> m_game;
	Guild* m_guild = nullptr;
	friend void from_json(const nlohmann::json& in, guild_member& out);
	friend struct shard;
};

void from_json(const nlohmann::json& in, guild_member& out);

void to_json(nlohmann::json& out, const guild_member& in);


