#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "permision_overwrite.h"

class shard;

class Channel{
public:
	snowflake id()const { return m_id; }
	const std::string& name()const noexcept { return m_name; }
private:
	std::string m_name = "";
	snowflake m_id;
	friend class shard;
	shard* m_shard = nullptr;
	friend void from_json(const nlohmann::json& json, Channel& channel);	
};

inline void to_json(nlohmann::json& json,const Channel& ch) {
	json["id"] = ch.id();
	json["name"] = ch.name();
}

inline void from_json(const nlohmann::json& json, Channel& channel) {
	channel.m_id = json["id"].get<snowflake>();
	channel.m_name = json.value("name","");
}
