#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "shard.h"
#include <chrono>
#include <thread>
#include "discord_enums.h"
#include "guild.h"
#include "text_channel.h"
#include "dm_channel.h"
#include "timed_task_executor.h"
#include "optional_ref.h"


using namespace std::literals;

enum class token_type {
	BOT,
	BEARER
};

static constexpr auto nothing = [](auto&&...) {};//so i don't get std::bad_function

/*
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

static inline constexpr empty_function_t empty_function;
*/
struct client {//<(^.^)>
	explicit client(int = 1);
	client(client&&) = delete;
	client(const client&) = delete;
	client& operator=(client&&) = delete;
	client& operator=(const client&) = delete;
	~client() = default;
	void run();

	void setToken(token_type type, std::string token);	

	void set_up_request(boost::beast::http::request<boost::beast::http::string_body>& req) const;

	std::string_view token()const { return m_token; }
		
	size_t num_shards()const noexcept { return m_num_shards; }
	std::function<void(const guild_text_message&, shard&)> on_guild_text_msg = nothing;
	std::function<void(const dm_message&, shard&)> on_dm_msg = nothing;
	std::function<void(const text_channel&, shard&)> on_guild_text_channel_create = nothing;
	std::function<void(const channel_catagory&, shard&)> on_guild_channel_catagory_create = nothing;
	std::function<void(const voice_channel&, shard&)> on_guild_voice_channel_create = nothing;
	std::function<void(const dm_channel&, shard&)> on_dm_channel_create = nothing;
	std::function<void(const guild_member&, shard&)> on_guild_member_add = nothing;
	std::function<void(const guild_member&, bool, shard&)> on_guild_member_remove = nothing;
	std::function<void(const text_channel&, shard&)> on_guild_text_channel_update = nothing;
	std::function<void(const dm_channel&, shard&)> on_dm_channel_update = nothing;
	std::function<void(const voice_channel&, shard&)> on_guild_voice_channel_update = nothing;
	std::function<void(const channel_catagory&, shard&)> on_guild_channel_catagory_update = nothing;
	std::function<void(const Guild&, shard&)> on_guild_update = nothing;
	std::function<void(const Guild&, bool, shard&)> on_guild_remove = nothing;
	std::function<void(const Guild&, const user&, shard&)> on_ban_add = nothing;
	std::function<void(const Guild&, const user&, shard&)> on_ban_remove = nothing;
	std::function<void(const guild_member&, shard&)> on_guild_member_update = nothing;
	std::function<void(const Guild&, const guild_role&, shard&)> on_role_create = nothing;
	std::function<void(const Guild&, const guild_role&, const guild_role&, shard&)> on_role_update = nothing;
	std::function<void(const Guild&, const guild_role&, shard&)> on_role_delete = nothing;
	std::function<void(const dm_msg_update&, optional_ref<const dm_message>, shard&)> on_dm_msg_update = nothing;
	std::function<void(const guild_msg_update&, optional_ref<const guild_text_message>, shard&)> on_guild_msg_update = nothing;
	std::function<void(std::optional<dm_message>,snowflake, shard&)> on_dm_msg_delete = nothing;
	std::function<void(std::optional<guild_text_message>,snowflake, shard&)> on_guild_msg_delete = nothing;
	std::function<void(const guild_member&, const text_channel&, shard&)> on_guild_typing_start = nothing;
	std::function<void(const user&, const dm_channel&, shard&)> on_dm_typing_start = nothing;
	std::function<void(const text_channel&, shard&)> on_text_channel_delete = nothing;
	std::function<void(const dm_channel&, shard&)> on_dm_channel_delete = nothing;
	std::function<void(const guild_member&, const text_channel&, optional_ref<guild_text_message>, const reaction&,shard&)> on_guild_reaction_add = nothing;
	std::function<void(const user&, const dm_channel&, optional_ref<dm_message>, const reaction&, shard&)> on_dm_reaction_add = nothing;
	std::function<void(const guild_member&, const text_channel&, optional_ref<guild_text_message>, const reaction&, shard&)> on_guild_reaction_remove = nothing;
	std::function<void(const user&, const dm_channel&, optional_ref<dm_message>, reaction&, shard&)> on_dm_reaction_remove = nothing;
	std::function<void(const text_channel&, optional_ref<const guild_text_message>, shard&)> on_guild_reaction_remove_all = nothing;
	std::function<void(const dm_channel&, optional_ref<const dm_message>, shard&)> on_dm_reaction_remove_all = nothing;
	std::function<void(const guild_member&, shard&)> on_presence_update = nothing;
	std::function<void(std::vector<snowflake>, const text_channel&, shard&)> on_message_bulk = nothing;
	std::function<void(std::vector<snowflake>, const dm_channel&, shard&)> on_dm_message_bulk = nothing;
	
	void rate_limit_global(const std::chrono::system_clock::time_point);

	Status status = Status::online;	
	std::string gameName = "";
 	timed_task_executor heartbeat_sender;

	/*
	void add_shard(shard* sh) {
		std::lock_guard a(m_shard_mut);
		m_shards_vec.push_back(sh);
	}
	*/

	void stop();

private:	
	void m_getGateway();
	std::chrono::system_clock::time_point m_last_global_rate_limit = std::chrono::system_clock::now();
	std::string m_endpoint;
		
	std::string m_authToken;
	std::string m_gateway = ""s;
	size_t m_num_shards = 0;
	std::string m_token = "";
	//
	std::mutex m_global_rate_limit_mut;

	std::mutex m_shard_mut;
	//std::vector<shard*> m_shards_vec;
	std::vector<std::unique_ptr<shard>> m_shards;
	std::vector<std::thread> m_threads;
	boost::asio::io_context m_ioc{};	
};



