#pragma once
#include "snowflake.h"
#include "intents.h"
#include <boost/asio.hpp>
#include <variant>
#include "internal_shard.h"
#include "unavailable_guild.h"
#include "guild_member_update.h"
#include "events.h"


namespace cacheless {

enum class token_type {
	BOT,
	BEARER
};

static constexpr auto nothing = [](auto&&...) {};//so i don't get std::bad_function

struct client {
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

	std::optional<std::function<void(events::guild_message_create, shard&)>> on_guild_text_msg;
	std::optional<std::function<void(events::dm_message_create, shard&)>> on_dm_msg;

	std::optional<std::function<void(events::text_channel_create, shard&)>> on_guild_text_channel_create;
	std::optional<std::function<void(events::channel_catagory_create, shard&)>> on_guild_channel_catagory_create;
	std::optional<std::function<void(events::voice_channel_create, shard&)>> on_guild_voice_channel_create;
	std::optional<std::function<void(events::dm_channel_create, shard&)>> on_dm_channel_create;

	std::optional<std::function<void(events::text_channel_update, shard&)>> on_guild_text_channel_update;
	std::optional<std::function<void(events::dm_channel_update, shard&)>> on_dm_channel_update;
	std::optional<std::function<void(events::voice_channel_update, shard&)>> on_guild_voice_channel_update;
	std::optional<std::function<void(events::channel_catagory_update, shard&)>> on_guild_channel_catagory_update;

	std::optional<std::function<void(events::text_channel_delete, shard&)>> on_guild_text_channel_delete;
	std::optional<std::function<void(events::dm_channel_delete, shard&)>> on_dm_channel_delete;
	std::optional<std::function<void(events::voice_channel_delete, shard&)>> on_guild_voice_channel_delete;
	std::optional<std::function<void(events::channel_catagory_delete, shard&)>> on_guild_channel_catagory_delete;

	std::optional<std::function<void(events::guild_member_add, shard&)>> on_guild_member_add;
	std::optional<std::function<void(events::guild_member_remove, shard&)>> on_guild_member_remove;

	std::optional<std::function<void(events::guild_member_update, shard&)>> on_guild_member_update;
	std::optional<std::function<void(events::guild_members_chunk, shard&)>> on_guild_member_chunk;

	std::optional<std::function<void(events::guild_update, shard&)>> on_guild_update;
	std::optional<std::function<void(events::guild_delete, shard&)>> on_guild_remove;
	std::optional<std::function<void(events::guild_ban_add, shard&)>> on_ban_add;
	std::optional<std::function<void(events::guild_ban_remove, shard&)>> on_ban_remove;
	std::optional<std::function<void(events::guild_role_create, shard&)>> on_role_create;
	std::optional<std::function<void(events::guild_role_update, shard&)>> on_role_update;
	std::optional<std::function<void(events::guild_role_delete, shard&)>> on_role_delete;

	std::optional<std::function<void(events::dm_message_update, shard&)>> on_dm_msg_update;
	std::optional<std::function<void(events::guild_message_update, shard&)>> on_guild_msg_update;

	std::optional<std::function<void(events::dm_message_delete, shard&)>> on_dm_msg_delete;
	std::optional<std::function<void(events::guild_message_delete, shard&)>> on_guild_msg_delete;

	std::optional<std::function<void(events::guild_typing_start, shard&)>> on_guild_typing_start;
	std::optional<std::function<void(events::dm_typing_start, shard&)>> on_dm_typing_start;

	std::optional<std::function<void(events::guild_message_reaction_add, shard&)>> on_guild_reaction_add;
	std::optional<std::function<void(events::dm_message_reaction_add, shard&)>> on_dm_reaction_add;

	std::optional<std::function<void(events::guild_message_reaction_remove, shard&)>> on_guild_reaction_remove;
	std::optional<std::function<void(events::dm_message_reaction_remove, shard&)>> on_dm_reaction_remove;

	std::optional<std::function<void(events::guild_message_reaction_remove_all, shard&)>> on_guild_reaction_remove_all;
	std::optional<std::function<void(events::dm_message_reaction_remove_all, shard&)>> on_dm_reaction_remove_all;

	std::optional<std::function<void(events::presence_update_event, shard&)>> on_presence_update;

	std::optional<std::function<void(events::guild_create, shard&)>> on_guild_ready;

	std::optional<std::function<void(events::guild_message_delete_bulk, shard&)>> on_message_bulk_delete;
	std::optional<std::function<void(events::dm_message_delete_bulk, shard&)>> on_dm_message_bulk_delete;
	std::optional<std::function<void(shard&)>> on_ready = nothing;
	std::optional<std::function<void(events::guild_emoji_update, shard&)>> on_emoji_update;

	void stop();

	virtual void rate_limit_global(std::chrono::system_clock::time_point);

	boost::asio::io_context& context() {
		if (std::holds_alternative<boost::asio::io_context>(m_ioc)) {
			return std::get<boost::asio::io_context>(m_ioc);
		} else {
			return *std::get<boost::asio::io_context*>(m_ioc);
		}
	}

	virtual std::chrono::steady_clock::time_point get_time_point_for_identifying() {
		//use mutex or atomic?

		std::lock_guard lock(m_identify_mut);
		//5.1s to account for some random delay that might happen? 
		m_last_identify = std::max(std::chrono::steady_clock::now(), m_last_identify + std::chrono::milliseconds(5100));
		return m_last_identify;
	}

	virtual void queue_to_identify(internal_shard& s) {
		std::lock_guard lock(m_identify_mut);
		if (m_no_one_is_identifying) {
			m_no_one_is_identifying = false;
			auto timer = std::make_unique<boost::asio::steady_timer>(context());
			timer->expires_at(m_last_identify + 5s);
			timer->async_wait([pin = std::move(timer), s_ = &s](auto ec) {
				if (ec) {}
				else {
					(void)s_->send_identity();
				}
			});

		}
		else {
			m_identifying_stack.push_back(&s);
		}
	}

	virtual void notify_identified() {
		std::lock_guard lock(m_identify_mut);
		m_last_identify = std::chrono::steady_clock::now();

		if (!m_identifying_stack.empty()) {
			const auto next = m_identifying_stack.back();
			m_identifying_stack.pop_back();
			auto timer = std::make_unique<boost::asio::steady_timer>(context());
			timer->expires_at(m_last_identify + 5s);
			timer->async_wait([pin = std::move(timer), s_ = next](auto ec) {
				if (ec) {}
				else {
					(void)s_->send_identity();
				}
			});
		}
		else {
			m_no_one_is_identifying = true;
		}
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
	bool m_no_one_is_identifying = true;
	std::vector<internal_shard*> m_identifying_stack;
	

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


}
