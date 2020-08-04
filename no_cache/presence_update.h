#pragma once
#include "snowflake.h"
#include "discord_enums.h"
#include <optional>
#include "User.h"
#include "timestamp.h"

namespace cacheless {
enum struct activity_type {
	game,
	streaming,
	listening
};

struct activity {
	std::optional<std::string> url;
	activity_type type = activity_type::game;
	std::string name;


	
	friend void from_json(const nlohmann::json& json, activity& thing);
};

constexpr int jaskdghasjd = sizeof(activity);

void from_json(const nlohmann::json& json, activity& thing);

struct partial_presence_update {
	snowflake id;
	Status status = Status::dnd;
	std::optional<activity> game;
	friend void from_json(const nlohmann::json&, partial_presence_update&);
};


void from_json(const nlohmann::json& json, partial_presence_update& thing);

struct client_status {
	std::optional<std::string> desktop;
	std::optional<std::string> mobile;
	std::optional<std::string> web;
};

inline void from_json(const nlohmann::json& json,client_status& status) {
	if(json.contains("desktop")) {
		status.desktop = json["desktop"].get<std::string>();
	}
	if (json.contains("mobile")) {
		status.mobile = json["mobile"].get<std::string>();
	}
	if (json.contains("web")) {
		status.web = json["web"].get<std::string>();
	}
}

struct presence_update_event {
	user user;
	std::vector<snowflake> roles;
	std::optional<activity> game;
	snowflake guild_id;
	Status status;
	std::vector<activity> activities;
	client_status client_status;
	std::optional<timestamp> premium_since;
	std::optional<std::string> nick;
	
};

inline void from_json(const nlohmann::json& json, presence_update_event& e) {
	json["user"].get_to(e.user);
	json["roles"].get_to(e.roles);
	json["game"].get_to(e.roles);
	json["guild_id"].get_to(e.roles);
	json["status"].get_to(e.roles);
	json["activities"].get_to(e.roles);
	json["client_status"].get_to(e.roles);
	if(json.contains("premium_since")) {
		e.premium_since = json["premium_since"].get<timestamp>();
	}

	if(json.contains("nick")) {
		e.nick = json["nick"].get<std::string>();
	}

	
}

};
