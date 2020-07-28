#pragma once
#include <nlohmann/json.hpp>
#include "snowflake.h"
#include <range/v3/core.hpp>
#include <span>
#include <fmt/core.h>
#include <fmt/format.h>


struct partial_emoji {
	partial_emoji() = default;
	
	explicit partial_emoji(std::string name):m_name(std::move(name)){}
	
	snowflake id() const noexcept { return m_id; };

	std::string_view name() const noexcept { return m_name; };

	std::string to_string() const {
		if (m_id == snowflake(0)) {
			return ":" + m_name + ":";
		}
		if (m_animated) {
			return fmt::format("<a:{}:{}>", m_name, m_id.val);
		} else {
			return fmt::format("<:{}:{}>", m_name, m_id.val);
		}
	}

	std::string to_reaction_string() const {
		if (m_id == snowflake(0)) {
			return ":" + m_name + ":";
		}
		if (m_animated) {
			return fmt::format("a:{}:{}", m_name, m_id.val);
		} else {
			return fmt::format(":{}:{}", m_name, m_id.val);
		}
	}

private:
	snowflake m_id = snowflake(0);
	std::string m_name;
	bool m_animated = false;
	friend void from_json(const nlohmann::json&, partial_emoji&);
};

struct emoji :partial_emoji {
	snowflake user_id() const noexcept { return m_user_id; }

	std::span<const snowflake> role_ids() const noexcept {
		return m_roles;
	}

private:
	std::vector<snowflake> m_roles;
	snowflake m_user_id;
};

inline void from_json(const nlohmann::json& json, partial_emoji& e) {
	e.m_id = json.value("id", snowflake(0));
	e.m_name = json.value("name", std::string(""));
	e.m_animated = json.value("animated", false);
}

inline void from_json(const nlohmann::json& json, emoji& e) {
	from_json(json, static_cast<partial_emoji&>(e));
}
