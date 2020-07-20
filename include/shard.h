#pragma once
#include <nlohmann/json.hpp>
#include "snowflake.h"
#include "guild.h"
#include "dm_channel.h"
#include "voice_channel.h"
#include "channel_catagory.h"
#include "partial_message.h"
#include "requests.h"
#include "http_connection.h"
#include <type_traits>
#include "range-like-stuffs.h"
#include "discord_enums.h"
#include "task.h"
#include "rename_later_5.h"
#include "attachment.h"
#include "discord_voice_connection.h"
#include "modify_guild_settings.h"
#include "modify_role_settings.h"
#include "intents.h"
#include "webhook_client.h"
#include "allowed_mentions.h"

struct shard {
protected:

	explicit shard(int shard_number, client* t_parent, boost::asio::io_context& ioc,std::string auth_token);
	
public:
	
	//content
	//content, embed
	//content, attachment
	//content, embed,attachment
	//have optional allowed_mentions?
	rq::send_message send_message(const partial_channel& channel, std::string content);

	rq::send_message send_message(const partial_channel& channel, std::string content, const embed& embed);
	
	template<int flags>
	rq::send_message send_message(const partial_channel& channel, std::string content,const allowed_mentions<flags>&);

	template<int flags>
	rq::send_message send_message(const partial_channel& channel, std::string content, const embed& embed, const allowed_mentions<flags>&);

	
	rq::add_role add_role(const partial_guild&, const guild_member&, const guild_role&);
	rq::remove_role remove_role(const partial_guild&, const guild_member&, const guild_role&);
	rq::modify_member remove_all_roles(const partial_guild&, const guild_member&);

	rq::create_role create_role(const partial_guild&, std::string, permission, int color = 0xffffff/*white*/, bool hoist = true, bool mentionable = true);
	rq::delete_role delete_role(const partial_guild&, const guild_role&);

	rq::modify_member change_nick(const guild_member&, std::string);
	rq::modify_member change_nick(const partial_guild&, const user&, std::string);
	rq::change_my_nick change_my_nick(const partial_guild&, std::string);
	rq::kick_member kick_member(const partial_guild&, const guild_member&);
	rq::ban_member ban_member(const partial_guild& g, const guild_member& member, std::string reason = "", int days_to_delete_msg = 0);
	rq::unban_member unban_member(const Guild&, snowflake id);
	//max 100 messages
	rq::get_messages get_messages(const partial_channel&, int = 100);
	rq::get_messages get_messages_before(const partial_channel&, snowflake, int = 100);
	rq::get_messages get_messages_before(const partial_channel&, const partial_message&, int = 100);
	rq::get_messages get_messages_after(const partial_channel&, snowflake, int = 100);
	rq::get_messages get_messages_after(const partial_channel&, const partial_message&, int = 100);
	rq::get_messages get_messages_around(const partial_channel&, snowflake, int = 100);
	rq::get_messages get_messages_around(const partial_channel&, const partial_message&, int = 100);
	rq::create_text_channel create_text_channel(const Guild&, std::string, std::vector<permission_overwrite>  = {}, bool = false);
	rq::edit_message edit_message(const partial_message&, std::string);
	rq::create_voice_channel create_voice_channel(const Guild&, std::string, std::vector<permission_overwrite>  = {}, bool = false, int = 96);
	rq::create_channel_catagory create_channel_catagory(const Guild&, std::string, std::vector<permission_overwrite>  = {}, bool = false);
	rq::delete_emoji delete_emoji(const partial_guild&, const emoji&);
	rq::modify_emoji modify_emoji(Guild&, emoji&, std::string, std::vector<snowflake>);
	rq::delete_message delete_message(const partial_message&);

	template<typename rng>
	requires is_range_of_v<rng, partial_message>
	rq::delete_message_bulk delete_message_bulk(const partial_channel&, rng&&);

	template<typename rng>
	requires is_range_of_v<rng, snowflake>
	rq::delete_message_bulk delete_message_bulk(const partial_channel&, rng&&);


	rq::delete_message_bulk delete_message_bulk(const partial_channel&, std::vector<snowflake>);
	rq::leave_guild leave_guild(const Guild&);
	rq::add_reaction add_reaction(const partial_message&, const partial_emoji&);
	rq::delete_own_reaction delete_own_reaction(const partial_message& msg, const partial_emoji&);
	rq::delete_own_reaction delete_own_reaction(const partial_message& msg, const reaction&);
	rq::delete_user_reaction delete_user_reaction(const partial_message& msg, const partial_emoji&,const user&);
	rq::delete_user_reaction delete_user_reaction(const partial_message& msg, const reaction&, const user&);
	rq::get_reactions get_reactions(const partial_message& msg, const partial_emoji&);
	rq::get_reactions get_reactions(const partial_message& msg,const reaction&);
	rq::delete_all_reactions delete_all_reactions(const partial_message& msg);
	rq::delete_all_reactions_emoji delete_all_reactions_emoji(const partial_message& msg, const partial_emoji&);
	rq::delete_all_reactions_emoji delete_all_reactions_emoji(const partial_message& msg, const reaction&);
	
	rq::typing_start typing_start(const partial_channel&);
	rq::delete_channel_permission delete_channel_permissions(const partial_guild_channel&, const permission_overwrite&);

	template<typename rng>
	rq::modify_channel_positions modify_channel_positions(const Guild&, rng&&);

	rq::list_guild_members list_guild_members(const partial_guild&, int n = 1, snowflake after = {});
	rq::edit_channel_permissions edit_channel_permissions(const partial_guild_channel&, const permission_overwrite&);
	rq::create_dm create_dm(const user&);

	rq::get_guild_integrations get_guild_integrations(const partial_guild& guild);

	rq::create_guild_integration create_guild_integration(const partial_guild& guild, std::string type, snowflake id);


	template<typename range>
	requires is_range_of<range, std::string>
	rq::create_group_dm create_group_dm(range&& access_tokens, std::unordered_map<snowflake, std::string> nicks = {});

	template<typename rng>
	requires is_range_of<rng, snowflake>
	rq::add_guild_member add_guild_member(const Guild& guild, snowflake id, std::string access_token, rng&& roles, std::string nick = "", bool deaf = false, bool mute = false);


	template<typename rng>
	requires is_range_of<rng, guild_role>
	rq::add_guild_member add_guild_member(const Guild& guild, snowflake id, std::string access_token, rng&& roles, std::string nick = "", bool deaf = false, bool mute = false);

	rq::delete_channel delete_channel(const partial_channel&);
	rq::add_pinned_msg add_pinned_msg(const partial_channel&, const partial_message&);
	rq::remove_pinned_msg remove_pinned_msg(const partial_channel&, const partial_message&);
	rq::get_voice_regions get_voice_regions();
	rq::create_channel_invite create_channel_invite(const partial_guild_channel&, int max_age = 86400, int max_uses = 0, bool temporary = false, bool unique = false);

	rq::get_invite get_invite(std::string, int = 0);
	rq::delete_invite delete_invite(std::string);

	template<typename ...settings>
	rq::modify_guild modify_guild(const partial_guild&, settings&&...);

	template<typename ...settings>
	rq::modify_guild modify_guild(const partial_guild&, modify_guild_settings<settings...>);

	template<typename... settings>
	rq::modify_role modify_role(const guild_role&, settings&&...);

	template<typename... settings>
	rq::modify_role modify_role(const guild_role&, modify_role_settings<settings...>);

	rq::create_webhook create_webhook(const partial_channel& channel, std::string name);

	rq::get_guild_webhooks get_guild_webhooks(const partial_guild&);

	rq::get_channel_webhooks get_channel_webhooks(const partial_channel&);

	rq::get_webhook get_webhook(snowflake, std::string token);

	rq::get_webhook get_webhook(const webhook&);

	rq::execute_webhook send_with_webhook(const webhook&, std::string s);


	template<typename ...settings>
	rq::modify_webhook modify_webhook(const webhook&, modify_webhook_settings<settings...>);

	template<typename ...settings>
	rq::modify_webhook modify_webhook(const webhook&, settings&&...);

	rq::get_message fetch_message(const partial_channel& ch, snowflake msg_id);

	rq::get_audit_log get_audit_log(
		const partial_guild&
	);

	const user& self_user() const noexcept {
		return m_self_user;
	}

	cerwy::task<voice_connection> connect_voice(const voice_channel&);

	boost::asio::io_context::strand& strand() {
		return m_strand;
	}

	discord_obj_map<Guild> guilds() const noexcept { return m_guilds; }

	discord_obj_map<text_channel> text_channels() const noexcept { return m_text_channel_map; }

	discord_obj_map<dm_channel> dm_channels() const noexcept { return m_dm_channels; }

	discord_obj_map<voice_channel> voice_channels() const noexcept { return m_voice_channel_map; }

	discord_obj_map<channel_catagory> channel_catagories() const noexcept { return m_channel_catagory_map; }

	webhook_client make_webhook_client(const webhook& wh) {
		if (!wh.token()) {
			throw std::runtime_error(";-;");
		}

		return webhook_client(wh.id(), std::string(wh.token().value()), m_strand);
	}

protected:
	using wsClient = rename_later_5;

	cerwy::task<boost::beast::error_code> connect_http_connection();

	void set_up_request(boost::beast::http::request<boost::beast::http::string_body>&) const;

	template<typename T, typename ... args>
	std::enable_if_t<rq::has_content_type_v<T>, T> send_request(std::string&&, args&&...);

	template<typename T, typename ... args>
	std::enable_if_t<!rq::has_content_type_v<T>, T> send_request(args&&...);


	int m_shard_number = 0;

	user m_self_user;


	http_connection2 m_http_connection;
	

	boost::asio::io_context::strand m_strand;

	client* m_parent = nullptr;

	std::string m_auth_token;


	ref_stable_map<snowflake, Guild> m_guilds;
	ref_stable_map<snowflake, text_channel> m_text_channel_map;
	ref_stable_map<snowflake, voice_channel> m_voice_channel_map;
	ref_stable_map<snowflake, channel_catagory> m_channel_catagory_map;
	ref_stable_map<snowflake, dm_channel> m_dm_channels;

	bool will_have_guild(snowflake guild_id) const noexcept;

	friend cerwy::task<void> init_shard(int shardN, internal_shard& t_parent, boost::asio::io_context& ioc, std::string_view gateway);
	friend struct client;
};

namespace rawrland {//rename later ;-;

	template<typename fut_type, typename ... Args>
	std::pair<fut_type, discord_request> get_default_stuffs_for_request(Args&&... args) {
		std::pair<fut_type, discord_request> retVal;
		retVal.second.state = make_ref_count_ptr<rq::shared_state>();

		retVal.first = fut_type(retVal.second.state);
		retVal.second.req = fut_type::request(std::forward<Args>(args)...);

		if constexpr (rq::has_content_type_v<fut_type>) {
			retVal.second.req.set(boost::beast::http::field::content_type, fut_type::content_type);
		}
		return retVal;
	}

}


template<typename T, typename ... args>
std::enable_if_t<rq::has_content_type_v<T>, T> shard::send_request(std::string&& body, args&&... Args) {
	auto [retVal, r] = rawrland::get_default_stuffs_for_request<T>(std::forward<args>(Args)...);
	set_up_request(r.req);
	r.req.body() = std::move(body);
	r.req.prepare_payload();
	r.state->strand = &m_strand;
	m_http_connection.send(std::move(r));
	return std::move(retVal);//no nrvo cuz structured bindings
}

template<typename T, typename ... args>
std::enable_if_t<!rq::has_content_type_v<T>, T> shard::send_request(args&&... Args) {
	auto [retVal, r] = rawrland::get_default_stuffs_for_request<T>(std::forward<args>(Args)...);
	set_up_request(r.req);
	r.req.prepare_payload();
	r.state->strand = &m_strand;
	m_http_connection.send(std::move(r));
	return std::move(retVal);//no nrvo cuz structured bindings
}

template<int flags>
rq::send_message shard::send_message(const partial_channel& channel, std::string content, const allowed_mentions<flags>& mentions_allowed) {
	nlohmann::json body = {};
	body["content"] = std::move(content);
	body["allowed_mentions"] = mentions_allowed;
	
	return send_request<rq::send_message>(body.dump(), channel);;
}

template<int flags>
rq::send_message shard::send_message(const partial_channel& channel, std::string content, const embed& embed, const allowed_mentions<flags>& mentions_allowed) {
	nlohmann::json body = {};
	body["content"] = std::move(content);
	body["allowed_mentions"] = mentions_allowed;
	body["embed"] = embed;

	return send_request<rq::send_message>(body.dump(), channel);;
}

template<typename rng>
rq::modify_channel_positions shard::modify_channel_positions(const Guild& g, rng&& positions) {
	nlohmann::json json =
			positions |
			ranges::views::transform([](auto&& a) {
				auto [id, position] = a;

				nlohmann::json ret;
				ret["id"] = id;
				ret["position"] = position;
				return ret;
			}) | ranges::to<std::vector>();

	return send_request<rq::modify_channel_positions>(json.dump(), g);
}

template<typename rng>
requires is_range_of_v<rng, partial_message>

rq::delete_message_bulk shard::delete_message_bulk(const partial_channel& channel, rng&& msgs) {
	nlohmann::json body;
	body["messages"] = msgs | ranges::views::transform(&partial_message::id) | ranges::to<std::vector>;
	return send_request<rq::delete_message_bulk>(body.dump(), channel);
}

template<typename rng>
requires is_range_of_v<rng, snowflake>

rq::delete_message_bulk shard::delete_message_bulk(const partial_channel& channel, rng&& msgs) {
	nlohmann::json body;
	if constexpr (std::is_convertible_v<rng, nlohmann::json>) {
		body["messages"] = msgs;
	} else {
		body["messages"] = msgs | ranges::to<std::vector>;
	}
	nlohmann::json body;
	body["messages"] = msgs | ranges::views::transform(&partial_message::id) | ranges::to<std::vector>;
	return send_request<rq::delete_message_bulk>(body.dump(), channel);
}

template<typename rng>
requires is_range_of<rng, snowflake>

rq::add_guild_member shard::add_guild_member(const Guild& guild, snowflake id, std::string access_token, rng&& roles, std::string nick, bool deaf, bool mute) {
	nlohmann::json body;
	body["access_token"] = access_token;
	body["nick"] = std::move(nick);
	body["deaf"] = deaf;
	body["mute"] = mute;
	if constexpr (std::is_convertible_v<rng, nlohmann::json>) {
		body["roles"] = roles;
	} else {
		std::vector<snowflake> stuff = roles | ranges::to<std::vector<snowflake>>();
		body["roles"] = std::move(stuff);
	}
	return send_request<rq::add_guild_member>(body.dump(), guild, id);
}

template<typename rng>
requires is_range_of<rng, guild_role>

rq::add_guild_member shard::add_guild_member(const Guild& guild, snowflake id, std::string access_token, rng&& roles, std::string nick, bool deaf, bool mute) {
	nlohmann::json body;
	body["access_token"] = access_token;
	body["nick"] = std::move(nick);
	body["deaf"] = deaf;
	body["mute"] = mute;
	body["roles"] = roles | ranges::views::transform(&guild_role::id) | ranges::to<std::vector>();

	return send_request<rq::add_guild_member>(body.dump(), guild, id);
}

template<typename ... settings>
rq::modify_guild shard::modify_guild(const partial_guild& g, settings&&... setting) {
	nlohmann::json body;
	((body[setting.vname] = std::move(setting.n)), ...);
	return send_request<rq::modify_guild>(body.dump(), g);
}

template<typename ... settings>
rq::modify_guild shard::modify_guild(const partial_guild& g, modify_guild_settings<settings...> s) {
	nlohmann::json body;
	std::apply([&](auto&&... setting) {
		((body[setting.vname] = std::move(setting.n)), ...);
	}, std::move(s.stuff));
	return send_request<rq::modify_guild>(body.dump(), g);
}

template<typename ... settings>
rq::modify_role shard::modify_role(const guild_role& g, settings&&... setting) {
	nlohmann::json body;
	((body[setting.vname] = std::move(setting.n)), ...);
	return send_request<rq::modify_role>(body.dump(), g);
}

template<typename ... settings>
rq::modify_role shard::modify_role(const guild_role& g, modify_role_settings<settings...> s) {
	nlohmann::json body;
	std::apply([&](auto&&... setting) {
		((body[setting.vname] = std::move(setting.n)), ...);
	}, std::move(s.stuff));
	return send_request<rq::modify_role>(body.dump(), g);
}

template<typename ... settings>//requires sizeof...(settings)>=1
rq::modify_webhook shard::modify_webhook(const webhook& wh, ::modify_webhook_settings<settings...> settings_) {
	return std::apply([&](auto&& ... settings__) {
			return modify_webhook(wh,std::forward<decltype(settings__)>(settings__)...);
		},std::move(settings_.settings)
	);
}

template<typename ... settings>//requires sizeof...(settings)>=1
rq::modify_webhook shard::modify_webhook(const webhook& wh, settings&&... s) {
	nlohmann::json body = {};
	((body[s.vname] = std::move(s.n)), ...);
	return send_request<rq::modify_webhook>(body.dump(), wh);
}


template<typename range>
requires is_range_of<range, std::string>

rq::create_group_dm shard::create_group_dm(range&& access_tokens, std::unordered_map<snowflake, std::string> nicks) {
	nlohmann::json body;
	body["access_tokens"] = std::vector<int>();
	for (auto&& token : access_tokens) {
		body["access_tokens"].push_back(token);
	}
	body["nicks"] = std::move(nicks);
	return send_request<rq::create_group_dm>(body.dump());
}
