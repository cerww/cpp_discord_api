#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "internal_shard.h"
#include <chrono>
#include <thread>
#include "discord_enums.h"
#include "guild.h"
#include "text_channel.h"
#include "dm_channel.h"
#include "../common/optional_ref.h"
#include "intents.h"


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
	explicit client(int = 1, intents = intent::ALL);
	
	explicit client(boost::asio::io_context&, intents = intent::ALL);
	
	client(client&&) = delete;
	client(const client&) = delete;
	client& operator=(client&&) = delete;
	client& operator=(const client&) = delete;
	virtual ~client() = default;
	
	virtual void run();

	void set_token(std::string token, token_type type = token_type::BOT);

	
	//void set_up_request(boost::beast::http::request<boost::beast::http::string_body>& req) const;

	std::string_view token() const { return m_token; }

	std::string_view auth_token() const { return m_authToken; }

	size_t num_shards() const noexcept { return m_num_shards; }

	std::function<void(guild_text_message, shard&)> on_guild_text_msg = nothing;
	std::function<void(dm_message, shard&)> on_dm_msg = nothing;
	std::function<void(const text_channel&, shard&)> on_guild_text_channel_create = nothing;
	std::function<void(const channel_catagory&, shard&)> on_guild_channel_catagory_create = nothing;
	std::function<void(const voice_channel&, shard&)> on_guild_voice_channel_create = nothing;
	std::function<void(const dm_channel&, shard&)> on_dm_channel_create = nothing;
	
	std::function<void(const guild_member&, shard&)> on_guild_member_add = nothing;
	std::function<void(const guild_member&, shard&)> on_guild_member_remove = nothing;
	std::function<void(const guild_member&, shard&)> on_guild_member_update = nothing;
	
	std::function<void(const text_channel&, shard&)> on_guild_text_channel_update = nothing;
	std::function<void(const dm_channel&, shard&)> on_dm_channel_update = nothing;
	std::function<void(const voice_channel&, shard&)> on_guild_voice_channel_update = nothing;
	std::function<void(const channel_catagory&, shard&)> on_guild_channel_catagory_update = nothing;
	std::function<void(const Guild&, shard&)> on_guild_update = nothing;
	std::function<void(const Guild&, bool, shard&)> on_guild_remove = nothing;
	std::function<void(const Guild&, user, shard&)> on_ban_add = nothing;
	std::function<void(const Guild&, user, shard&)> on_ban_remove = nothing;
	std::function<void(const Guild&, const guild_role&, shard&)> on_role_create = nothing;
	std::function<void(const Guild&, guild_role, const guild_role&, shard&)> on_role_update = nothing;
	std::function<void(const Guild&, guild_role, shard&)> on_role_delete = nothing;
	std::function<void(dm_msg_update,  shard&)> on_dm_msg_update = nothing;
	std::function<void(guild_msg_update, shard&)> on_guild_msg_update = nothing;
	std::function<void(snowflake, const dm_channel&, shard&)> on_dm_msg_delete = nothing;
	std::function<void(snowflake, const text_channel&, shard&)> on_guild_msg_delete = nothing;
	std::function<void(guild_member, const text_channel&, shard&)> on_guild_typing_start = nothing;
	std::function<void(const user&, const dm_channel&, shard&)> on_dm_typing_start = nothing;
	
	std::function<void(const text_channel&, shard&)> on_text_channel_delete = nothing;
	std::function<void(const voice_channel&, shard&) > on_voice_channel_delete = nothing;
	std::function<void(const channel_catagory&, shard&)> on_channel_catagory_delete = nothing;
	std::function<void(const dm_channel&, shard&)> on_dm_channel_delete = nothing;
	
	std::function<void(guild_member, const text_channel&, snowflake, partial_emoji, shard&)> on_guild_reaction_add = nothing;
	std::function<void(const user&, const dm_channel&, snowflake, partial_emoji, shard&)> on_dm_reaction_add = nothing;
	
	std::function<void(snowflake/*member_id*/, const text_channel&, snowflake/*msg_id*/, partial_emoji, shard&)> on_guild_reaction_remove = nothing;
	std::function<void(const user&, const dm_channel&, snowflake,partial_emoji, shard&)> on_dm_reaction_remove = nothing;
	
	std::function<void(const text_channel&, snowflake,  shard&)> on_guild_reaction_remove_all = nothing;
	std::function<void(const dm_channel&, snowflake,  shard&)> on_dm_reaction_remove_all = nothing;
	
	std::function<void(const guild_member&, shard&)> on_presence_update = nothing;

	std::function<void(const Guild&, shard&)> on_guild_ready = nothing;
	
	std::function<void(std::vector<snowflake>, const text_channel&, shard&)> on_message_bulk = nothing;
	std::function<void(std::vector<snowflake>, const dm_channel&, shard&)> on_dm_message_bulk = nothing;
	std::function<void(shard&)> on_ready = nothing;



	//Status status = Status::online;
	//timed_task_executor heartbeat_sender;

	void stop();


	virtual void rate_limit_global(std::chrono::system_clock::time_point);

	boost::asio::io_context& context() {
		if(std::holds_alternative<boost::asio::io_context>(m_ioc)) {
			return std::get<boost::asio::io_context>(m_ioc);
		}else {
			return *std::get<boost::asio::io_context*>(m_ioc);
		}
	}
	
	virtual std::chrono::steady_clock::time_point get_time_point_for_identifying() {
		//use mutex or atomic?
		
		std::lock_guard lock(m_identify_mut);
		//5.1s to account for some random delay that might happen 
		m_last_identify = std::max(std::chrono::steady_clock::now(),m_last_identify + std::chrono::milliseconds(5100));
		return m_last_identify;
	}	

	virtual void do_gateway_stuff();

	client& set_shards(size_t s) {
		assert(s > 0);
		m_num_shards = s;
		return *this;
	}
	
protected:
	void m_getGateway();
	std::chrono::system_clock::time_point m_last_global_rate_limit = std::chrono::system_clock::now();
	//1 identify every 5s, -6s is so we don't wait 5s for the first one
	std::chrono::steady_clock::time_point m_last_identify = std::chrono::steady_clock::now() - std::chrono::seconds(6);
	std::mutex m_identify_mut;

	intents m_intents = {};

	
	std::string m_endpoint;

	std::string m_authToken;
	std::string m_gateway = ""s;
	size_t m_num_shards = 0;
	std::string m_token = "";
	//
	std::mutex m_global_rate_limit_mut;

	std::mutex m_shard_mut;
	std::vector<std::unique_ptr<internal_shard>> m_shards;
	std::vector<std::thread> m_threads;

	std::variant<boost::asio::io_context, boost::asio::io_context*> m_ioc{};

	
};
