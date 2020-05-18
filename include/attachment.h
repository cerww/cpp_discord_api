#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "snowflake.h"


struct attachment {
	snowflake id() const { return m_id; }

	std::string_view filename() const { return m_filename; }

	int size() const { return m_size; }

	std::string_view url() const { return m_url; }

	std::string_view proxy_url() const { return m_proxy_url; }

	int width() const { return m_width; }

	int height() const { return m_height; }

private:
	snowflake m_id;
	std::string m_filename;
	int m_size = 0;
	std::string m_url;
	std::string m_proxy_url;
	int m_width = 0;
	int m_height = 0;
	friend void from_json(const nlohmann::json& json, attachment& val);
};

inline void from_json(const nlohmann::json& json, attachment& val) {
	val.m_id = json["id"].get<snowflake>();
	val.m_filename = json["filename"].get<std::string>();
	val.m_size = json["size"].get<int>();
	val.m_url = json["url"].get<std::string>();
	val.m_proxy_url = json["proxy_url"].get<std::string>();
}
