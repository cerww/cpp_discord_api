#include "shard.h"
#include "snowflake.h"
#include "guild.h"

const guild_member& Guild::owner() const noexcept {
	return m_members.at(owner_id());
}

timestamp Guild::joined_at() const noexcept { return m_joined_at; }

bool Guild::large() const noexcept { return m_large; }

bool Guild::unavailable() const noexcept { return m_unavailable; }

int Guild::member_count() const noexcept { return m_member_count; }

discord_obj_map<text_channel> Guild::all_text_channels() const noexcept {
	return m_shard->text_channels();
}

discord_obj_map<voice_channel> Guild::all_voice_channels() const noexcept {
	return m_shard->voice_channels();
}

discord_obj_map<channel_catagory> Guild::all_channel_catagories() const noexcept {
	return m_shard->channel_catagories();
}

void from_json(const nlohmann::json& json, Guild& guild) {
	from_json(json, static_cast<partial_guild&>(guild));
	guild.m_large = json["large"].get<bool>();
	guild.m_unavailable = json["unavailable"].get<bool>();
	guild.m_member_count = json["member_count"].get<int>();
	guild.m_members.reserve(guild.m_member_count);
	for(const auto& member_json:json["members"]) {
		auto member = member_json.get<guild_member>();
		const auto id = member.id();
		guild.m_members.insert(std::make_pair(id,std::move(member)));
	}
	guild.m_voice_states = json["voice_states"].get<std::vector<voice_state>>();
}

const text_channel& Guild::general_channel() const noexcept {
	return m_shard->text_channels()[general_channel_id()];
}

optional_ref<voice_channel> Guild::afk_channel() const noexcept {
	if (afk_channel_id().val)
		return m_shard->voice_channels().at(afk_channel_id());
	return std::nullopt;		
}