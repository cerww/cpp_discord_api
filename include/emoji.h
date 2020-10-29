#pragma once
#include <nlohmann/json.hpp>
#include <span>
#include <fmt/core.h>
#include "snowflake.h"
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
			//fmt::print("{}\n", nlohmann::json(m_name).dump().substr(1, m_name.size()));
			return ":" + nlohmann::json(m_name).dump().substr(1,m_name.size()) + ":";
		}
		if (m_animated) {
			return fmt::format("a:{}:{}", m_name, m_id.val);
		} else {
			return fmt::format(":{}:{}", m_name, m_id.val);
		}
	}

	bool is_custom()const noexcept {
		return m_id != snowflake(0);
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
	//fmt::print("{}\n", json.dump());
	e.m_id = json.value("id", snowflake(0));
	e.m_name = json.value("name", std::string(""));	
	e.m_animated = json.value("animated", false);
}

inline void from_json(const nlohmann::json& json, emoji& e) {
	from_json(json, static_cast<partial_emoji&>(e));
}

template<typename Char>
struct fmt::formatter<partial_emoji,Char>:formatter<std::string_view,Char> {

	template <typename FormatContext>
	auto format(const partial_emoji& val, FormatContext& ctx) {
		return formatter<string_view,Char>::format(val.to_string(), ctx);
	}
	
};
template<typename Char>
struct fmt::formatter<emoji, Char> :formatter<partial_emoji, Char> {
		
};