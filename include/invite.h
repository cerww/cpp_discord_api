#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include "snowflake.h"
#include "User.h"

namespace invite_{

struct partial_guild{
	snowflake id;
	std::string name;
	std::string splash;
	std::string icon;
};

struct partial_channel{
	snowflake id;
	std::string name;
	int type;
};

inline void from_json(const nlohmann::json& json,partial_guild& g) {
	g.id = json["id"].get<snowflake>();
	g.name = json["name"].get<std::string>();
	g.splash = json["splash"].get<std::string>();
	g.icon = json["icon"].get<std::string>();
}

inline void from_json(const nlohmann::json& json, partial_channel& g) {
	g.id = json["id"].get<snowflake>();
	g.name = json["name"].get<std::string>();
	g.type = json["type"].get<int>();
}

}

struct invite{
	const std::string& code()const noexcept { return m_code; }
	const invite_::partial_guild& guild()const noexcept { return  m_guild; }
	const invite_::partial_channel& channel()const noexcept { return  m_channel; }
private:
	std::string m_code;
	invite_::partial_guild m_guild;
	invite_::partial_channel m_channel;
	friend void from_json(const nlohmann::json& j, invite& i);
};


struct invite_metadata{
private:
	std::optional<User> m_user;
	int m_uses = 0;
	int m_max_uses = 0;
	friend void from_json(const nlohmann::json& j, invite_metadata& i);
}; 

inline void from_json(const nlohmann::json& j,invite& i) {
	i.m_code = j["code"].get<std::string>();
	i.m_guild = j["guild"].get<invite_::partial_guild>();
	i.m_channel = j["channel"].get<invite_::partial_channel>();
}

inline void from_json(const nlohmann::json& j, invite_metadata& stuff) {

	stuff.m_max_uses = j["max_uses"].get<int>();
}
