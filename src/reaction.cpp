#include "reaction.h"


int reaction::count() const noexcept { return m_count; }

bool reaction::me() const noexcept { return m_me; }

const partial_emoji& reaction::emoji() const noexcept { return m_emoji; }

void from_json(const nlohmann::json& json, reaction& r) {
	r.m_count = json["count"].get<int>();
	r.m_me = json["me"].get<bool>();
	r.m_emoji = json["emoji"].get<partial_emoji>();
}
