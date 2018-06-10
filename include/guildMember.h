#pragma once
#include <vector>
#include "user.h"
#include "guild_role.h"
#include <nlohmann/json.hpp>
#include "timestamp.h"
#include <optional>
#include "presence_update.h"
#include "partial_guild_member.h"

class Guild;

class guild_member:public partial_guild_member{
public:
	Guild& guild() noexcept;;
	const Guild& guild() const noexcept;;
	discord_obj_list<guild_role> roles()const;
	Status status() const noexcept;;
private:
	void set_presence(const partial_presence_update& presence_update);

	Status m_status = Status::online;
	std::optional<activity> m_game;
	Guild* m_guild = nullptr;

	friend void from_json(const nlohmann::json& in, guild_member& out);
	friend class shard;
};

void from_json(const nlohmann::json& in, guild_member& out);

void to_json(nlohmann::json& out, const guild_member& in);


