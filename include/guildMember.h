#pragma once
#include <vector>
#include "User.h"
#include "Role.h"
#include <nlohmann/json.hpp>
#include "timestamp.h"
#include <optional>
#include "presence_update.h"

class Guild;

class guild_member:public User{
public:
	Guild& guild() noexcept;;
	const Guild& guild() const noexcept;;
	const std::string& nick() const noexcept;

	thing<Role> roles()const;

	const std::vector<snowflake>& role_ids() const noexcept;
	timestamp joined_at() const noexcept;
	bool deaf() const noexcept;
	bool mute() const noexcept;
	bool has_role(const snowflake role_id) const noexcept;

	bool has_role(const Role& role) const noexcept;

	bool has_role(Role&& role) const noexcept;
	Status status() const noexcept;;
private:
	std::string m_nick;
	std::vector<snowflake> m_roles;
	timestamp m_joined_at;
	bool m_deaf = false;
	bool m_mute = false;
	snowflake m_guild_id;
	Status m_status = Status::online;
	std::optional<activity> m_game;

	Guild* m_guild = nullptr;
	void set_presence(const partial_presence_update& presence_update);

	friend void from_json(const nlohmann::json& in, guild_member& out);
	friend class shard;
};

void from_json(const nlohmann::json& in, guild_member& out);

void to_json(nlohmann::json& out, const guild_member& in);
