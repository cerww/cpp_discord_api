#pragma once
#include "snowflake.h"
#include <nlohmann/json.hpp>

struct discord_object{
	snowflake id()const noexcept { return m_id; }
private:
	snowflake m_id;
	friend void from_json(const nlohmann::json&, discord_object&);
};

inline void from_json(const nlohmann::json& json, discord_object& thing) {
	thing.m_id = json.at("id").get<snowflake>();
}

/*
template<typename T>
concept bool discord_object = requires(T a){
	{a.id()}->snowflake;
	{from_json(const nlohmann::json&,a)}->void;
	
}

*/
