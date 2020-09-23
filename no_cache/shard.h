#pragma once
#include "snowflake.h"
#include "http_connection.h"
#include "discord_voice_connection.h"
#include "allowed_mentions.h"
#include "modify_guild_settings.h"
#include "rename_later_5.h"
#include "modify_role_settings.h"



namespace cacheless {

struct shard {

protected:

	explicit shard(int shard_number, client* t_parent, boost::asio::io_context& ioc, std::string auth_token);

public:

	//content
	//content, embed
	//content, attachment
	//content, embed,attachment
	//have optional allowed_mentions?
	rq::send_message send_message(snowflake channel, std::string content);

	rq::send_message send_message(snowflake channel, std::string content, const embed& embed);

	template<int flags>
	rq::send_message send_message(snowflake channel, std::string content, const allowed_mentions<flags>&);

	template<int flags>
	rq::send_message send_message(snowflake channel, std::string content, const embed& embed, const allowed_mentions<flags>&);


	rq::add_role add_role(snowflake guild_id, snowflake member_id, snowflake role_id);
	rq::remove_role remove_role(snowflake guild_id, snowflake member_id, snowflake role_id);
	rq::modify_member remove_all_roles(snowflake guild_id, snowflake member_id);

	rq::create_role create_role(snowflake guild_id, std::string name, permission, int color = 0xffffff/*white*/, bool hoist = true, bool mentionable = true);
	rq::delete_role delete_role(snowflake guild_id, snowflake role_id);

	rq::modify_member change_nick(snowflake guild_id, snowflake member_id, std::string);
	rq::change_my_nick change_my_nick(snowflake guild_id, std::string);
	rq::kick_member kick_member(snowflake guild_id, snowflake member_id);
	rq::ban_member ban_member(snowflake guild_id, snowflake member_id, std::string reason = "", int days_to_delete_msg = 0);
	rq::unban_member unban_member(snowflake guild_id, snowflake member_id);
	//max 100 messages
	rq::get_messages get_messages(snowflake channel_id, int = 100);
	rq::get_messages get_messages_before(snowflake channel_id, snowflake, int = 100);
	rq::get_messages get_messages_after(snowflake channel_id, snowflake, int = 100);
	rq::get_messages get_messages_around(snowflake channel_id, snowflake, int = 100);
	rq::create_text_channel create_text_channel(snowflake guild_id, std::string, std::vector<permission_overwrite>  = {}, bool = false);
	rq::edit_message edit_message(snowflake channel_id, snowflake msg_id, std::string);
	rq::create_voice_channel create_voice_channel(snowflake guild_id, std::string, std::vector<permission_overwrite>  = {}, bool = false, int = 96);
	rq::create_channel_catagory create_channel_catagory(snowflake guild_id, std::string, std::vector<permission_overwrite>  = {}, bool = false);
	rq::delete_emoji delete_emoji(snowflake guild_id, const partial_emoji&);
	rq::modify_emoji modify_emoji(snowflake guild_id, const partial_emoji&, std::string, std::vector<snowflake>);
	rq::delete_message delete_message(snowflake channel_id, snowflake msg_id);

	template<typename rng>
	requires is_range_of_v<rng, snowflake>
	rq::delete_message_bulk delete_message_bulk(snowflake channel_id, rng&&);

	rq::delete_message_bulk delete_message_bulk(snowflake channel_id, std::vector<snowflake>);
	rq::leave_guild leave_guild(snowflake guild_id);
	rq::add_reaction add_reaction(snowflake channel_id, snowflake msg_id, const partial_emoji&);
	rq::delete_own_reaction delete_own_reaction(snowflake channel_id,snowflake msg_id, const partial_emoji&);
	rq::delete_own_reaction delete_own_reaction(snowflake channel_id, snowflake msg_id, const reaction&);
	rq::delete_user_reaction delete_user_reaction(snowflake channel_id, snowflake msg_id, const partial_emoji&, const snowflake&);
	rq::delete_user_reaction delete_user_reaction(snowflake channel_id, snowflake msg_id, const reaction&, const snowflake&);
	rq::get_reactions get_reactions(snowflake channel_id, snowflake msg_id, const partial_emoji&);
	rq::get_reactions get_reactions(snowflake channel_id, snowflake msg_id, const reaction&);
	rq::delete_all_reactions delete_all_reactions(snowflake channel_id, snowflake msg_id);
	rq::delete_all_reactions_emoji delete_all_reactions_emoji(snowflake channel_id, snowflake msg_id, const partial_emoji&);
	rq::delete_all_reactions_emoji delete_all_reactions_emoji(snowflake channel_id, snowflake msg_id, const reaction&);

	rq::typing_start typing_start(snowflake channel_id);
	rq::delete_channel_permission delete_channel_permissions(snowflake channel_id, const permission_overwrite&);

	template<typename rng>
	rq::modify_channel_positions modify_channel_positions(snowflake guild_id, rng&&);

	rq::list_guild_members list_guild_members(snowflake guild_id, int n = 1, snowflake after = {});
	rq::edit_channel_permissions edit_channel_permissions(snowflake channel_id, const permission_overwrite&);
	rq::create_dm create_dm(const user&);
	rq::get_guild_integrations get_guild_integrations(snowflake guild_id );
	rq::create_guild_integration create_guild_integration(snowflake guild_id, std::string type, snowflake id);
	
	template<typename range>
	requires is_range_of<range, std::string>
	rq::create_group_dm create_group_dm(range&& access_tokens, std::unordered_map<snowflake, std::string> nicks = {});

	template<typename rng>
	requires is_range_of<rng, snowflake>
	rq::add_guild_member add_guild_member(snowflake guild_id, snowflake id, std::string access_token, rng&& roles, std::string nick = "", bool deaf = false, bool mute = false);

	rq::delete_channel delete_channel(snowflake channel_id );
	rq::add_pinned_msg add_pinned_msg(snowflake channel_id, snowflake msg_id);
	rq::remove_pinned_msg remove_pinned_msg(snowflake channel_id, snowflake msg_id);
	rq::get_voice_regions get_voice_regions();
	rq::create_channel_invite create_channel_invite(snowflake channel_id, int max_age = 86400, int max_uses = 0, bool temporary = false, bool unique = false);

	rq::get_invite get_invite(std::string, int = 0);
	rq::delete_invite delete_invite(std::string);

	template<typename ...settings>
	rq::modify_guild modify_guild(snowflake guild_id, settings&&...);

	template<typename ...settings>
	rq::modify_guild modify_guild(snowflake guild_id, modify_guild_settings<settings...>);

	template<typename... settings>
	rq::modify_role modify_role(snowflake role_id, settings&&...);

	template<typename... settings>
	rq::modify_role modify_role(snowflake role_id, modify_role_settings<settings...>);

	rq::create_webhook create_webhook(snowflake channel_id, std::string name);
	rq::get_guild_webhooks get_guild_webhooks(snowflake guild_id );
	rq::get_channel_webhooks get_channel_webhooks(snowflake channel_id);
	rq::get_webhook get_webhook(snowflake, std::string token);
	rq::get_webhook get_webhook(const webhook&);
	rq::execute_webhook send_with_webhook(const webhook&, std::string s);

	template<typename ...settings>
	rq::modify_webhook modify_webhook(const webhook&, modify_webhook_settings<settings...>);

	template<typename ...settings>
	rq::modify_webhook modify_webhook(const webhook&, settings&&...);

	rq::get_message fetch_message(snowflake channel_id, snowflake msg_id);

	rq::get_audit_log get_audit_log(snowflake guild_id);

	snowflake id() const noexcept{
		return m_id;
	}

	virtual cerwy::task<voice_connection> connect_voice(const voice_channel&) = 0;
	virtual cerwy::task<voice_connection> connect_voice(snowflake, snowflake) = 0;

	boost::asio::io_context::strand& strand() {
		return m_strand;
	}

	virtual void request_guild_members(snowflake g) const = 0;

protected:
	using wsClient = rename_later_5;

	

	void set_up_request(boost::beast::http::request<boost::beast::http::string_body>&) const;

	template<typename T, typename ... args>
	std::enable_if_t<rq::has_content_type_v<T>, T> send_request(std::string&&, args&&...);

	template<typename T, typename ... args>
	std::enable_if_t<!rq::has_content_type_v<T>, T> send_request(args&&...);


	int m_shard_number = 0;

	//user m_self_user;
	snowflake m_id;


	http_connection2 m_http_connection;


	boost::asio::io_context::strand m_strand;

	client* m_parent_client = nullptr;

	std::string m_auth_token;

	async_mutex m_events_mut;


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
rq::send_message shard::send_message(snowflake channel, std::string content, const allowed_mentions<flags>& mentions_allowed) {
	nlohmann::json body = {};
	body["content"] = std::move(content);
	body["allowed_mentions"] = mentions_allowed;

	return send_request<rq::send_message>(body.dump(), channel);;
}

template<int flags>
rq::send_message shard::send_message(snowflake channel, std::string content, const embed& embed, const allowed_mentions<flags>& mentions_allowed) {
	nlohmann::json body = {};
	body["content"] = std::move(content);
	body["allowed_mentions"] = mentions_allowed;
	body["embed"] = embed;

	return send_request<rq::send_message>(body.dump(), channel);;
}

template<typename rng>
rq::modify_channel_positions shard::modify_channel_positions(snowflake g, rng&& positions) {
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
requires is_range_of_v<rng, snowflake>

rq::delete_message_bulk shard::delete_message_bulk(snowflake channel, rng&& msgs) {
	nlohmann::json body;
	if constexpr (std::is_convertible_v<rng, nlohmann::json>) {
		body["messages"] = msgs;
	}
	else {
		body["messages"] = msgs | ranges::to<std::vector>;
	}
	nlohmann::json body;
	body["messages"] = msgs | ranges::views::transform(&partial_message::id) | ranges::to<std::vector>;
	return send_request<rq::delete_message_bulk>(body.dump(), channel);
}

template<typename rng>
requires is_range_of<rng, snowflake>

rq::add_guild_member shard::add_guild_member(snowflake guild, snowflake id, std::string access_token, rng&& roles, std::string nick, bool deaf, bool mute) {
	nlohmann::json body;
	body["access_token"] = std::move(access_token);
	body["nick"] = std::move(nick);
	body["deaf"] = deaf;
	body["mute"] = mute;
	if constexpr (std::is_convertible_v<rng, nlohmann::json>) {
		body["roles"] = roles;
	}
	else {
		std::vector<snowflake> stuff = roles | ranges::to<std::vector<snowflake>>();
		body["roles"] = std::move(stuff);
	}
	return send_request<rq::add_guild_member>(body.dump(), guild, id);
}


template<typename ... settings>
rq::modify_guild shard::modify_guild(snowflake g, settings&&... setting) {
	nlohmann::json body;
	((body[setting.vname] = std::move(setting.n)), ...);
	return send_request<rq::modify_guild>(body.dump(), g);
}

template<typename ... settings>
rq::modify_guild shard::modify_guild(snowflake g, modify_guild_settings<settings...> s) {
	nlohmann::json body;
	std::apply([&](auto&&... setting) {
		((body[setting.vname] = std::move(setting.n)), ...);
		}, std::move(s.stuff));
	return send_request<rq::modify_guild>(body.dump(), g);
}

template<typename ... settings>
rq::modify_role shard::modify_role(snowflake g, settings&&... setting) {
	nlohmann::json body;
	((body[setting.vname] = std::move(setting.n)), ...);
	return send_request<rq::modify_role>(body.dump(), g);
}

template<typename ... settings>
rq::modify_role shard::modify_role(snowflake g, modify_role_settings<settings...> s) {
	nlohmann::json body;
	std::apply([&](auto&&... setting) {
		((body[setting.vname] = std::move(setting.n)), ...);
		}, std::move(s.stuff));
	return send_request<rq::modify_role>(body.dump(), g);
}

template<typename ... settings>//requires sizeof...(settings)>=1
rq::modify_webhook shard::modify_webhook(const webhook& wh, modify_webhook_settings<settings...> settings_) {
	return std::apply([&](auto&& ... settings__) {
		return modify_webhook(wh, std::forward<decltype(settings__)>(settings__)...);
		}, std::move(settings_.settings)
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

	// body["access_tokens"] = std::vector<int>();
	// for (auto&& token : access_tokens) {
	// 	body["access_tokens"].push_back(token);
	// }
	body["access_tokens"] = access_tokens | ranges::to<nlohmann::json>();
	body["nicks"] = std::move(nicks);
	return send_request<rq::create_group_dm>(body.dump());
}


}
