#pragma once
#include <nlohmann/json.hpp>
#include "emoji.h"

struct reaction{
	int count() const noexcept;;
	bool me() const noexcept;
	const partial_emoji& emoji() const noexcept;
private:
	int m_count = 0;
	bool m_me = false;
	partial_emoji m_emoji;

	friend void from_json(const nlohmann::json&, reaction&);
	friend struct shard;
};

void from_json(const nlohmann::json& json, reaction& r);
