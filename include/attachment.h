#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "snowflake.h"

struct attachment{
	snowflake id;
	std::string filename;
	int size = 0;
	std::string url;
	std::string proxy_url;
	int width = 0;
	int height = 0;
};

inline void from_json(const nlohmann::json& json,attachment& val) {
	val.id = json["id"].get<snowflake>();
	val.filename = json["filename"].get<std::string>();
	val.size = json["size"].get<int>();
	val.url = json["url"].get<std::string>();
	val.proxy_url = json["proxy_url"].get<std::string>();
}


