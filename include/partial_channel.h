#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "permision_overwrite.h"

class shard;

class partial_channel{
public:
	snowflake id()const { return m_id; }
	const std::string& name()const noexcept { return m_name; }
private:
	snowflake m_id;
	std::string m_name = "";

	friend class shard;
	friend void from_json(const nlohmann::json& json, partial_channel& channel);	
};

inline void to_json(nlohmann::json& json,const partial_channel& ch) {
	json["id"] = ch.id();
	json["name"] = ch.name();
}

inline void from_json(const nlohmann::json& json, partial_channel& channel) {
	channel.m_id = json["id"].get<snowflake>();
	channel.m_name = json.value("name","");
}
