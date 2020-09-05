#include "internal_shard.h"
#include "snowflake.h"
#include "guild.h"

const guild_member& Guild::owner() const {
	return m_members.at(owner_id());
}

timestamp Guild::joined_at() const noexcept { return m_joined_at; }

bool Guild::large() const noexcept { return m_large; }

bool Guild::unavailable() const noexcept { return m_unavailable; }

int Guild::member_count() const noexcept { return m_member_count; }



void from_json(const nlohmann::json& json, Guild& guild) {
	json.get_to(static_cast<partial_guild&>(guild));
	guild.m_large = json["large"].get<bool>();
	guild.m_unavailable = json["unavailable"].get<bool>();
	guild.m_member_count = json["member_count"].get<int>();
	guild.m_members.reserve(guild.m_member_count);
	guild.m_voice_states = json["voice_states"].get<std::vector<voice_state>>();
}

const text_channel& Guild::system_channel() const {
	return m_text_channels.at(system_channel_id());
}

optional_ref<const voice_channel> Guild::afk_channel() const {
	if (afk_channel_id().val) {
		return m_voice_channels.at(afk_channel_id());
	}
	return std::nullopt;
}
