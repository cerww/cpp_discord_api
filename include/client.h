#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "shard.h"
#include <chrono>
#include <thread>
#include "constant_stuffs.h"
#include "guild.h"
#include "text_channel.h"
#include "dm_channel.h"
#include "timed_task_executor.h"
#include "optional_ref.h"

using namespace std::string_literals;
using namespace std::chrono_literals;

enum class tokenType {
	BOT,
	BEARER
};

static const auto nothing = [](auto&&...) {};//so i don't get std::bad_function

struct empty_function_t{
	template<typename ret,typename...args>
	operator std::function<ret(args...)>()const noexcept{
		if constexpr(std::is_same_v<ret,void>){
			return [](args...) {};
		}else {
			return [](args...) ->ret{return {}; };
		}
	}
};

static inline empty_function_t empty_function;

class client {//<(^.^)>
public:
	client();
	client(client&&) = delete;
	client(const client&) = delete;
	client& operator=(client&&) = delete;
	client& operator=(const client&) = delete;
	~client() = default;
	void run();

	void setToken(tokenType type, std::string token);	

	void set_up_request(boost::beast::http::request<boost::beast::http::string_body>& req) const;

	nlohmann::json presence()const;
	const std::string& token()const { return m_token; }
	user& self_user() { return m_self; }
	size_t num_shards()const noexcept { return m_num_shards; }
	std::function<void(guild_text_message&, shard&)> on_guild_text_msg = nothing;
	std::function<void(dm_message&, shard&)> on_dm_msg = nothing;
	std::function<void(text_channel&, shard&)> on_guild_text_channel_create = nothing;
	std::function<void(channel_catagory&, shard&)> on_guild_channel_catagory_create = nothing;
	std::function<void(voice_channel&, shard&)> on_guild_voice_channel_create = nothing;
	std::function<void(dm_channel&, shard&)> on_dm_channel_create = nothing;
	std::function<void(guild_member&, shard&)> on_guild_member_add = nothing;
	std::function<void(guild_member&,bool,shard&)> on_guild_member_remove = nothing;
	std::function<void(text_channel&, shard&)> on_guild_text_channel_update = nothing;
	std::function<void(dm_channel&, shard&)> on_dm_channel_update = nothing;
	std::function<void(voice_channel&, shard&)> on_guild_voice_channel_update = nothing;
	std::function<void(channel_catagory&, shard&)> on_guild_channel_catagory_update = nothing;
	std::function<void(Guild&, shard&)> on_guild_update = nothing;
	std::function<void(const Guild&,bool, shard&)> on_guild_remove = nothing;
	std::function<void(Guild&, user&, shard&)> on_ban_add = nothing;
	std::function<void(Guild&, user&, shard&)> on_ban_remove = nothing;
	std::function<void(guild_member&, shard&)> on_guild_member_update = nothing;
	std::function<void(Guild&, guild_role&, shard&)> on_role_create = nothing;
	std::function<void(Guild&, guild_role&, guild_role&, shard&)> on_role_update = nothing;
	std::function<void(Guild&, const guild_role&, shard&)> on_role_delete = nothing;
	std::function<void(dm_msg_update&, optional_ref<dm_message>, shard&)> on_dm_msg_update = nothing; 
	std::function<void(guild_msg_update&, optional_ref<guild_text_message>, shard&)> on_guild_msg_update = nothing;
	std::function<void(std::optional<dm_message>,snowflake, shard&)> on_dm_msg_delete = nothing;
	std::function<void(std::optional<guild_text_message>,snowflake, shard&)> on_guild_msg_delete = nothing;
	std::function<void(guild_member&,text_channel&, shard&)> on_guild_typing_start = nothing;
	std::function<void(user&,dm_channel&, shard&)> on_dm_typing_start = nothing;
	std::function<void(text_channel&, shard&)> on_text_channel_delete = nothing;
	std::function<void(dm_channel&, shard&)> on_dm_channel_delete = nothing;
	std::function<void(guild_member&,text_channel&, optional_ref<guild_text_message>,reaction&,shard&)> on_guild_reaction_add = nothing;
	std::function<void(user&, dm_channel&, optional_ref<dm_message>, reaction&, shard&)> on_dm_reaction_add = nothing;
	std::function<void(guild_member&, text_channel&, optional_ref<guild_text_message>, reaction&, shard&)> on_guild_reaction_remove = nothing;
	std::function<void(user&, dm_channel&, optional_ref<dm_message>, reaction&, shard&)> on_dm_reaction_remove = nothing;
	std::function<void(text_channel&, optional_ref<guild_text_message>, shard&)> on_guild_reaction_remove_all = nothing;
	std::function<void(dm_channel&, optional_ref<dm_message>, shard&)> on_dm_reaction_remove_all = nothing;
	std::function<void(guild_member&, shard&)> on_presence_update = nothing;
	std::function<void(std::vector<snowflake>, text_channel&, shard&)> on_message_bulk = nothing;
	std::function<void(std::vector<snowflake>, dm_channel&, shard&)> on_dm_message_bulk = nothing;
	
	void rate_limit_global(const std::chrono::system_clock::time_point);
	void reconnect(shard* s,int shardNumber) {
		m_ws_hub.connect(m_gateway);
	}
	timed_task_executor heartbeat_sender;	
	Status status = Status::online;
	std::string gameName = "";
private:
	using wsClient = uWS::WebSocket<uWS::CLIENT>;
	void m_getGateway();
	std::chrono::system_clock::time_point m_last_global_rate_limit = std::chrono::system_clock::now();
	std::string m_endpoint;
		
	std::string m_authToken;
	std::string m_gateway = ""s;
	size_t m_num_shards = 0;
	std::string m_token = "";
	uWS::Hub m_ws_hub;
	//
	std::unordered_map<wsClient*, std::unique_ptr<shard>> m_shards = {};
	user m_self;	
};



