#include "shard.h"
#include "snowflake.h"
#include "guild.h"



thing<text_channel> Guild::text_channels()const {
	return thing<text_channel>(m_shard->text_channels(), m_text_channels);
}

thing<voice_channel> Guild::voice_channels()const {
	return thing<voice_channel>(m_shard->voice_channels(), m_voice_channels);
}

thing<channel_catagory> Guild::channel_catagories() const {
	return thing<channel_catagory>(m_shard->channel_catagories(), m_channel_catagories);
}

const std::vector<guild_member>& Guild::members() const noexcept { return m_members; }

std::vector<guild_member>& Guild::members() noexcept { return m_members; }

timestamp Guild::joined_at() const noexcept { return m_joined_at; }

bool Guild::large() const noexcept { return m_large; }

bool Guild::unavailable() const noexcept { return m_unavailable; }

int Guild::member_count() const noexcept { return m_member_count; }

const std::vector<voice_state>& Guild::voice_states() const noexcept { return m_voice_states; }

void from_json(const nlohmann::json& json, Guild& guild) {
	from_json(json, static_cast<partial_guild&>(guild));
	guild.m_large = json["large"].get<bool>();
	guild.m_unavailable = json["unavailable"].get<bool>();
	guild.m_member_count = json["member_count"].get<int>();
	guild.m_members = json["members"].get<std::vector<guild_member>>();
	guild.m_voice_states = json["voice_states"].get<std::vector<voice_state>>();
	std::sort(guild.m_members.begin(), guild.m_members.end(),
			  [](const auto& a, const auto& b){ return a.id().val < b.id().val; });
}

text_channel& Guild::general_channel() const noexcept {
	return m_shard->text_channels()[general_channel_id()];
}


voice_channel* Guild::afk_channel() const noexcept {
	if (afk_channel_id().val)
		return &m_shard->voice_channels().at(afk_channel_id());
	return nullptr;		
}