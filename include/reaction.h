#pragma once
#include <nlohmann/json.hpp>
#include "emoji.h"
class reaction{
public:
	int count() const noexcept;;
	bool me() const noexcept;
	const partial_emoji& emoji() const noexcept;
private:
	int m_count = 0;
	bool m_me = false;
	partial_emoji m_emoji;

	friend void from_json(const nlohmann::json&, reaction&);
	friend class shard;
};

void from_json(const nlohmann::json& json, reaction& r);
