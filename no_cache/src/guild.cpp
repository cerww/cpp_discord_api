#include "internal_shard.h"
#include "snowflake.h"
#include "guild.h"
#include "text_channel.h"

namespace cacheless {

void from_json(const nlohmann::json& json, Guild& guild) {
	from_json(json, static_cast<partial_guild&>(guild));
	guild.large = json["large"].get<bool>();
	guild.unavailable = json["unavailable"].get<bool>();
	guild.member_count = json["member_count"].get<int>();
	guild.members.reserve(guild.member_count);
	guild.voice_states = json["voice_states"].get<std::vector<voice_state>>();
	
	for(const auto& channel_json:json["channels"]) {
		const auto type = channel_json["type"].get<int>();
		if ( type == 0 || type == 5 || type == 6) {//text
			auto& channel = guild.text_channels.insert(std::make_pair(channel_json["id"].get<snowflake>(), channel_json.get<text_channel>())).first->second;
			
			channel.guild_id = guild.id;
			
			if (type == 5) {
				channel.channel_type = text_channel_type::news;
			}

			if (type == 6) {
				channel.channel_type = text_channel_type::store;
			}
		}
		else if (type == 2) {//voice
			auto& channel = guild.voice_channels.insert(std::make_pair(channel_json["id"].get<snowflake>(), channel_json.get<voice_channel>())).first->second;
			channel.guild_id = guild.id;
			//guild.m_voice_channel_ids.push_back(channel.id());
			//channel.m_guild = &guild;
		}
		else if (type == 4) {//guild catagory
			auto& channel = guild.channel_catagories.insert(std::make_pair(channel_json["id"].get<snowflake>(), channel_json.get<channel_catagory>())).first->second;
			channel.guild_id = guild.id;
			//guild.m_channel_catagory_ids.push_back(channel.id());
			//channel.m_guild = &guild;
		} else {
			//unimplemented channel
		}
	}
}


}
