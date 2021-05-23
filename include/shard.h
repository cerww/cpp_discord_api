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
#include "../common/range-like-stuffs.h"
#include "discord_enums.h"
#include "../common/eager_task.h"
#include "rename_later_5.h"
#include "attachment.h"
#include "discord_voice_connection.h"
#include "modify_guild_settings.h"
#include "modify_role_settings.h"
#include "intents.h"
#include "webhook_client.h"
#include "allowed_mentions.h"
#include "modifies_message_json.h"
#include "guild_text_message.h"
#include "dm_message.h"

namespace rawrland {//rename later ;-;

	template<typename request_type, typename ... Args>
	rq::request_data get_default_stuffs_for_request(Args&&... args) {

		rq::request_data ret;
		ret.req = request_type::request(std::forward<Args>(args)...);
		if constexpr (rq::has_content_type_v<request_type>) {
			ret.req.set(boost::beast::http::field::content_type, request_type::content_type);
		}


		return ret;
	}

}

struct shard {
protected:
	~shard() = default;

	explicit shard(int shard_number, client* t_parent, boost::asio::io_context& ioc, std::string auth_token);


public:
	// clang-format off
	//content
	[[nodiscard]] rq::send_message send_message(const partial_channel& channel, std::string content);
	[[nodiscard]] rq::send_message send_message(const partial_channel& channel, std::string content, const embed& embed);

	template<typename... modifiers>
	requires(message_modifier<modifiers>&&...)
	[[nodiscard]] rq::send_message send_message(const partial_channel& channel, std::string content, modifiers&&...);

	template<typename... modifiers>
	requires(message_modifier<modifiers>&&...)
	[[nodiscard]] rq::send_message send_message(const partial_channel& channel, std::string content, const embed& embed, modifiers&&...);
	
	[[nodiscard]] rq::send_message send_reply(const partial_message& msg, std::string content);
	[[nodiscard]] rq::send_message send_reply(const partial_message& msg, std::string content, const embed& embed);

	template<typename... modifiers>
	requires(message_modifier<modifiers>&&...)
	[[nodiscard]] rq::send_message send_reply(const partial_message& msg, std::string content, modifiers&&...);

	template<typename... modifiers>
	requires(message_modifier<modifiers>&&...)
	[[nodiscard]] rq::send_message send_reply(const partial_message& msg, std::string content, const embed& embed, modifiers&&...);

	
	[[nodiscard]] auto send_message(const guild_channel& channel, std::string content) {
		nlohmann::json body = {};
		body["content"] = std::move(content);

		return create_request<rq::send_guild_message>(body.dump(), channel);
	};
	
	[[nodiscard]] auto send_message(const guild_channel& channel, std::string content, const embed& embed) {
		nlohmann::json body = {};
		body["content"] = std::move(content);
		body["embed"] = embed;

		return create_request<rq::send_message>(body.dump(), channel);
	};

	template<typename... modifiers>
	requires(message_modifier<modifiers>&&...)
	[[nodiscard]] auto send_message(const guild_channel& channel, std::string content, modifiers&&... modifers_) {
		nlohmann::json body = {};
		body["content"] = std::move(content);
		(modifers_.modify_message_json(body), ...);

		return create_request<rq::send_message>(body.dump(), channel);;
	};

	template<typename... modifiers>
	requires(message_modifier<modifiers>&&...)
	[[nodiscard]] auto send_message(const guild_channel& channel, std::string content, const embed& embed, modifiers&&... modifers_) {
		nlohmann::json body = {};
		body["content"] = std::move(content);
		(modifers_.modify_message_json(body), ...);
		body["embed"] = embed;

		return create_request<rq::send_message>(body.dump(), channel);
	};
	

	[[nodiscard]] rq::add_role add_role(const partial_guild&, const partial_guild_member&, const guild_role&);
	[[nodiscard]] rq::remove_role remove_role(const partial_guild&, const partial_guild_member&, const guild_role&);

	[[nodiscard]] rq::add_role add_role(const guild_member&, const guild_role&);
	[[nodiscard]] rq::remove_role remove_role(const guild_member&, const guild_role&);
	[[nodiscard]] rq::modify_member remove_all_roles(const partial_guild&, const guild_member&);

	[[nodiscard]] rq::create_role create_role(const partial_guild&, std::string, permission, int color = 0xffffff/*white*/, bool hoist = true, bool mentionable = true);
	[[nodiscard]] rq::delete_role delete_role(const partial_guild&, const guild_role&);

	[[nodiscard]] rq::modify_member change_nick(const guild_member&, std::string new_nick);
	[[nodiscard]] rq::modify_member change_nick(const partial_guild&, const user&, std::string new_nick);

	[[nodiscard]] rq::modify_member assign_roles(const guild_member&, const std::vector<snowflake>& roles_ids);
	[[nodiscard]] rq::change_my_nick change_my_nick(const partial_guild&, std::string new_nick);
	[[nodiscard]] rq::kick_member kick_member(const partial_guild&, const partial_guild_member&);
	[[nodiscard]] rq::ban_member ban_member(const partial_guild& g, const partial_guild_member& member, std::string reason = "", int days_to_delete_msg = 0);
	[[nodiscard]] rq::unban_member unban_member(const partial_guild&, snowflake user_id);
	//max 100 messages
	[[nodiscard]] rq::get_messages get_messages(const partial_channel&, int = 100);
	[[nodiscard]] rq::get_messages get_messages_before(const partial_channel&, snowflake, int = 100);
	[[nodiscard]] rq::get_messages get_messages_before(const partial_channel&, const partial_message&, int = 100);
	[[nodiscard]] rq::get_messages get_messages_after(const partial_channel&, snowflake, int = 100);
	[[nodiscard]] rq::get_messages get_messages_after(const partial_channel&, const partial_message&, int = 100);
	[[nodiscard]] rq::get_messages get_messages_around(const partial_channel&, snowflake, int = 100);
	[[nodiscard]] rq::get_messages get_messages_around(const partial_channel&, const partial_message&, int = 100);
	[[nodiscard]] rq::create_text_channel create_text_channel(const Guild&, std::string name, const std::vector<permission_overwrite>& = {}, bool nsfw = false);
	[[nodiscard]] rq::edit_message edit_message(const partial_message&, std::string new_content);
	[[nodiscard]] rq::create_voice_channel create_voice_channel(const Guild&, std::string name, const std::vector<permission_overwrite>& = {}, bool nsfw = false, int bit_rate = 96);
	[[nodiscard]] rq::create_channel_catagory create_channel_catagory(const Guild&, std::string name, const std::vector<permission_overwrite>& = {}, bool nsfw = false);
	[[nodiscard]] rq::delete_emoji delete_emoji(const partial_guild&, const partial_emoji&);
	[[nodiscard]] rq::modify_emoji modify_emoji(const partial_guild&, const partial_emoji&, std::string name, const std::vector<snowflake>& role_ids);
	[[nodiscard]] rq::delete_message delete_message(const partial_message&);

	template<typename rng>
	requires is_range_of_v<rng, partial_message>
	[[nodiscard]] rq::delete_message_bulk delete_message_bulk(const partial_channel&, rng&&);

	template<typename rng>
	requires is_range_of_v<rng, snowflake>
	[[nodiscard]] rq::delete_message_bulk delete_message_bulk(const partial_channel&, rng&&);
	[[nodiscard]] rq::delete_message_bulk delete_message_bulk(const partial_channel&, const std::vector<snowflake>&);

	[[nodiscard]] rq::leave_guild leave_guild(const Guild&);
	[[nodiscard]] rq::add_reaction add_reaction(const partial_message&, const partial_emoji&);
	[[nodiscard]] rq::delete_own_reaction delete_own_reaction(const partial_message& msg, const partial_emoji&);
	[[nodiscard]] rq::delete_own_reaction delete_own_reaction(const partial_message& msg, const reaction&);
	[[nodiscard]] rq::delete_user_reaction delete_user_reaction(const partial_message& msg, const partial_emoji&, const user&);
	[[nodiscard]] rq::delete_user_reaction delete_user_reaction(const partial_message& msg, const reaction&, const user&);
	[[nodiscard]] rq::get_reactions get_reactions(const partial_message& msg, const partial_emoji&);
	[[nodiscard]] rq::get_reactions get_reactions(const partial_message& msg, const reaction&);
	[[nodiscard]] rq::delete_all_reactions delete_all_reactions(const partial_message& msg);
	[[nodiscard]] rq::delete_all_reactions_emoji delete_all_reactions_emoji(const partial_message& msg, const partial_emoji&);
	[[nodiscard]] rq::delete_all_reactions_emoji delete_all_reactions_emoji(const partial_message& msg, const reaction&);

	[[nodiscard]] rq::typing_start typing_start(const partial_channel&);
	[[nodiscard]] rq::delete_channel_permission delete_channel_permissions(const partial_guild_channel&, const permission_overwrite&);

	template<typename rng>
	[[nodiscard]] rq::modify_channel_positions modify_channel_positions(const Guild&, rng&&);

	[[nodiscard]] rq::list_guild_members list_guild_members(const partial_guild&, int n = 1, snowflake after = {});
	[[nodiscard]] rq::edit_channel_permissions edit_channel_permissions(const partial_guild_channel&, const permission_overwrite&);
	[[nodiscard]] rq::create_dm create_dm(const user&);

	[[nodiscard]] rq::get_guild_integrations get_guild_integrations(const partial_guild& guild);

	[[nodiscard]] rq::create_guild_integration create_guild_integration(const partial_guild& guild, std::string type, snowflake id);

	template<typename range>
	requires is_range_of<range, std::string>
	[[nodiscard]] rq::create_group_dm create_group_dm(range&& access_tokens, std::unordered_map<snowflake, std::string> nicks = {});

	template<typename rng>
	requires is_range_of<rng, snowflake>
	[[nodiscard]] rq::add_guild_member add_guild_member(const Guild& guild, snowflake id, std::string access_token, rng&& roles, std::string nick = "", bool deaf = false, bool mute = false);

	template<typename rng>
	requires is_range_of<rng, guild_role>
	[[nodiscard]] rq::add_guild_member add_guild_member(const Guild& guild, snowflake id, std::string access_token, rng&& roles, std::string nick = "", bool deaf = false, bool mute = false);

	[[nodiscard]] rq::delete_channel delete_channel(const partial_channel&);
	[[nodiscard]] rq::add_pinned_msg add_pinned_msg(const partial_channel&, const partial_message&);
	[[nodiscard]] rq::remove_pinned_msg remove_pinned_msg(const partial_channel&, const partial_message&);
	[[nodiscard]] rq::get_voice_regions get_voice_regions();
	[[nodiscard]] rq::create_channel_invite create_channel_invite(const partial_guild_channel&, int max_age = 86400, int max_uses = 0, bool temporary = false, bool unique = false);

	[[nodiscard]] rq::get_invite get_invite(std::string, int = 0);
	[[nodiscard]] rq::delete_invite delete_invite(std::string);

	template<typename ...settings>
	[[nodiscard]] rq::modify_guild modify_guild(const partial_guild&, settings&&...);

	template<typename ...settings>
	[[nodiscard]] rq::modify_guild modify_guild(const partial_guild&, modify_guild_settings<settings...>);

	template<typename... settings>
	[[nodiscard]] rq::modify_role modify_role(const guild_role&, settings&&...);

	template<typename... settings>
	[[nodiscard]] rq::modify_role modify_role(const guild_role&, modify_role_settings<settings...>);

	[[nodiscard]] rq::create_webhook create_webhook(const partial_channel& channel, std::string name);
	[[nodiscard]] rq::get_guild_webhooks get_guild_webhooks(const partial_guild&);
	[[nodiscard]] rq::get_channel_webhooks get_channel_webhooks(const partial_channel&);
	[[nodiscard]] rq::get_webhook get_webhook(snowflake, std::string token);
	[[nodiscard]] rq::get_webhook get_webhook(const webhook&);
	[[nodiscard]] rq::execute_webhook send_with_webhook(const webhook&, std::string s);

	template<typename ...settings>
	[[nodiscard]] rq::modify_webhook modify_webhook(const webhook&, modify_webhook_settings<settings...>);

	template<typename ...settings>
	[[nodiscard]] rq::modify_webhook modify_webhook(const webhook&, settings&&...);

	[[nodiscard]] rq::get_message fetch_message(const partial_channel& ch, snowflake msg_id);

	[[nodiscard]] rq::get_audit_log get_audit_log(const partial_guild&);

	const user& self_user() const noexcept {
		return m_self_user;
	}

	[[nodiscard]] virtual cerwy::eager_task<voice_connection> connect_voice(const voice_channel&) = 0;

	boost::asio::io_context::strand& strand() {
		return m_strand;
	}

	auto guilds() const noexcept {
		return map_transform(m_guilds,[this](const auto& a) {
			return std::as_const(*a->guild);
		});
	}

	webhook_client make_webhook_client(const webhook& wh) {
		if (!wh.token()) {
			throw std::runtime_error(";-;");
		}

		return webhook_client(wh.id(), std::string(wh.token().value()), m_strand);
	}

	bool will_have_guild(snowflake guild_id) const noexcept;

protected:
	using wsClient = rename_later_5;

	cerwy::eager_task<boost::beast::error_code> connect_http_connection();

	void set_up_request(boost::beast::http::request<boost::beast::http::string_body>&) const;

	template<typename T, typename ... args> requires rq::has_content_type_v<T>
	T create_request(std::string&&, args&&...);

	template<typename T, typename ... args>requires !rq::has_content_type_v<T>
	T create_request(args&&...);
	
	template<typename T>
	struct dependant_cb {
		dependant_cb() = delete;
		explicit dependant_cb(shard* a) :me(a) {}

		shard* me = nullptr;
	};

	//TODO change create_msg ;-;
	template<>
	struct dependant_cb<guild_text_message>{
		dependant_cb() = delete;
		explicit dependant_cb(shard* a):me(a){}

		auto operator()(std::string_view s) const{
			auto json = nlohmann::json::parse(s);
			const auto guild_id = json.value("guild_id", snowflake());
			const auto channel_id = json["channel_id"].get<snowflake>();
			auto& guild_and_stuff = me->m_guilds.at(guild_id);
			auto& guild = *guild_and_stuff->guild;

			const auto it = guild_and_stuff->text_channels.find(channel_id);

			if (it == guild_and_stuff->text_channels.end()) {
				throw std::runtime_error(";-;");
			}

			text_channel& ch = *it->second;
			return me->create_msg<guild_text_message>(ch, json);
		}
		
		shard* me = nullptr;
	};

	template<template<typename>typename T, typename ... args> requires rq::has_content_type_v<T<int>>
	auto create_request(std::string&& body, args&&... Args)  {
		auto r = rawrland::get_default_stuffs_for_request<T<int>>(std::forward<args>(Args)...);
		r.req.body() = std::move(body);
		r.req.prepare_payload();
		r.strand = &m_strand;
		r.http_connection = &m_http_connection;		
		set_up_request(r.req);
		return T(std::move(r), dependant_cb<typename T<int>::return_type>(this));
	}

	template<template<typename>typename T, typename ... args>requires !rq::has_content_type_v<T<int>>
	auto create_request(args&&... Args) {
		auto r = rawrland::get_default_stuffs_for_request<T<int>>(std::forward<args>(Args)...);
		r.req.prepare_payload();
		r.strand = &m_strand;
		r.http_connection = &m_http_connection;
		set_up_request(r.req);
		return T(std::move(r), dependant_cb<typename T<int>::return_type>(this));
	}

	guild_member make_member_from_msg(const nlohmann::json& user_json, const nlohmann::json& member_json) const;

	template<typename msg_t, typename channel_t>
	msg_t create_msg(channel_t&, const nlohmann::json&);


	int m_shard_number = 0;
	user m_self_user;
	http_connection2 m_http_connection;
	boost::asio::io_context::strand m_strand;
	client* m_parent_client = nullptr;
	std::string m_auth_token;

	ska::bytell_hash_map<snowflake, std::unique_ptr<guild_and_stuff>> m_guilds;
	
	ref_stable_map<snowflake, dm_channel> m_dm_channels;

	std::vector<std::pair<std::chrono::steady_clock::time_point, indirect<dm_channel>>> m_deleted_dm_channels;//TODO REMOVE

	friend cerwy::eager_task<void> init_shard(int shardN, internal_shard& t_parent, boost::asio::io_context& ioc, std::string_view gateway);
	friend struct client;
};


inline guild_member shard::make_member_from_msg(const nlohmann::json& user_json, const nlohmann::json& member_json) const {
	guild_member ret;
	user_json.get_to((user&)ret);

	const auto it = member_json.find("nick");
	if (it != member_json.end()) {
		ret.m_nick = it->is_null() ? "" : it->get<std::string>();
	}

	const auto& member_roles_json = member_json["roles"];
	ranges::push_back(ret.m_roles, member_roles_json | ranges::views::transform(&nlohmann::json::get<snowflake>));

	ret.m_deaf = member_json["deaf"].get<bool>();
	ret.m_mute = member_json["mute"].get<bool>();

	return ret;
}

template<typename msg_t, typename channel_t>
msg_t shard::create_msg(channel_t& ch, const nlohmann::json& stuffs) {
	msg_t retVal;
	stuffs.get_to(static_cast<partial_message&>(retVal));
	retVal.m_channel = &ch;

	constexpr bool is_guild_msg = std::is_same_v<msg_t, guild_text_message>;

	if constexpr (is_guild_msg) {
		if (stuffs.contains("webhook_id")) {
			from_json(stuffs["author"], static_cast<user&>(retVal.m_author));
			retVal.m_author.m_guild = ch.m_guild;
		}
		else {
			retVal.m_author = make_member_from_msg(stuffs["author"], stuffs["member"]);
			retVal.m_author.m_guild = ch.m_guild;
		}
	}
	else {
		retVal.m_author = stuffs["author"].get<user>();
	}

	if constexpr (is_guild_msg) {
		retVal.m_mentions.reserve(stuffs["mentions"].size());
		for (const auto& mention : stuffs["mentions"]) {
			auto& member = retVal.m_mentions.emplace_back(make_member_from_msg(mention, mention["member"]));
			member.m_guild = ch.m_guild;
		}
	}
	else {
		retVal.m_mentions = stuffs["mentions"].get<std::vector<user>>();
	}

	if constexpr (std::is_same_v<msg_t, guild_text_message>) {
		retVal.m_mention_roles_ids = stuffs["mention_roles"].get<lol_wat_vector<snowflake>>();
	}
	return retVal;
}



template<typename T, typename ... args>requires rq::has_content_type_v<T>
T shard::create_request(std::string&& body, args&&... Args) {
	auto r = rawrland::get_default_stuffs_for_request<T>(std::forward<args>(Args)...);
	r.req.body() = std::move(body);
	r.req.prepare_payload();
	r.strand = &m_strand;
	r.http_connection = &m_http_connection;
	set_up_request(r.req);	
	return T(std::move(r));
}

template<typename T, typename ... args>requires !rq::has_content_type_v<T>
T shard::create_request(args&&... Args) {
	auto r = rawrland::get_default_stuffs_for_request<T>(std::forward<args>(Args)...);	
	r.req.prepare_payload();
	r.strand = &m_strand;
	r.http_connection = &m_http_connection;
	set_up_request(r.req);
	return T(std::move(r));
}

template<typename... modifiers>
requires(message_modifier<modifiers>&&...)
rq::send_message shard::send_message(const partial_channel& channel, std::string content, modifiers&&... modifiers_) {
	nlohmann::json body = {};
	body["content"] = std::move(content);
	(modifiers_.modify_message_json(body), ...);

	return create_request<rq::send_message>(body.dump(), channel);;
}

template<typename... modifiers> requires(message_modifier<modifiers>&&...)
rq::send_message shard::send_message(const partial_channel& channel, std::string content, const embed& embed, modifiers&&... modifiers_) {
	nlohmann::json body = {};
	body["content"] = std::move(content);
	body["embed"] = embed;
	(modifiers_.modify_message_json(body), ...);

	return create_request<rq::send_message>(body.dump(), channel);;
}

template<typename rng>
rq::modify_channel_positions shard::modify_channel_positions(const Guild& g, rng&& positions) {
	const nlohmann::json json =
			positions |
			ranges::views::transform([](auto&& a) {
				auto [id, position] = a;

				nlohmann::json ret;
				ret["id"] = id;
				ret["position"] = position;
				return ret;
			}) | ranges::to<std::vector>();

	return create_request<rq::modify_channel_positions>(json.dump(), g);
}

template<typename rng>
requires is_range_of_v<rng, partial_message>
rq::delete_message_bulk shard::delete_message_bulk(const partial_channel& channel, rng&& msgs) {
	nlohmann::json body;
	body["messages"] = msgs | ranges::views::transform(&partial_message::id) | ranges::to<std::vector>;
	return create_request<rq::delete_message_bulk>(body.dump(), channel);
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
	return create_request<rq::delete_message_bulk>(body.dump(), channel);
}

template<typename rng>
requires is_range_of<rng, snowflake>
rq::add_guild_member shard::add_guild_member(const Guild& guild, snowflake id, std::string access_token, rng&& roles, std::string nick, bool deaf, bool mute) {
	nlohmann::json body;
	body["access_token"] = std::move(access_token);
	body["nick"] = std::move(nick);
	body["deaf"] = deaf;
	body["mute"] = mute;
	if constexpr (std::is_convertible_v<rng, nlohmann::json>) {
		body["roles"] = roles;
	} else {
		std::vector<snowflake> stuff = roles | ranges::to<std::vector<snowflake>>();
		body["roles"] = std::move(stuff);
	}
	return create_request<rq::add_guild_member>(body.dump(), guild, id);
}

template<typename rng>
requires is_range_of<rng, guild_role>
rq::add_guild_member shard::add_guild_member(const Guild& guild, snowflake id, std::string access_token, rng&& roles, std::string nick, bool deaf, bool mute) {
	nlohmann::json body;
	body["access_token"] = std::move(access_token);
	body["nick"] = std::move(nick);
	body["deaf"] = deaf;
	body["mute"] = mute;
	body["roles"] = roles | ranges::views::transform(&guild_role::id) | ranges::to<std::vector>();

	return create_request<rq::add_guild_member>(body.dump(), guild, id);
}

template<typename ... settings>
rq::modify_guild shard::modify_guild(const partial_guild& g, settings&&... setting) {
	nlohmann::json body;
	((body[setting.vname] = std::move(setting.n)), ...);
	return create_request<rq::modify_guild>(body.dump(), g);
}

template<typename ... settings>
rq::modify_guild shard::modify_guild(const partial_guild& g, modify_guild_settings<settings...> s) {
	nlohmann::json body;
	std::apply([&](auto&&... setting) {
		((body[setting.vname] = std::move(setting.n)), ...);
	}, std::move(s.stuff));
	return create_request<rq::modify_guild>(body.dump(), g);
}

template<typename ... settings>
rq::modify_role shard::modify_role(const guild_role& g, settings&&... setting) {
	nlohmann::json body;
	((body[setting.vname] = std::move(setting.n)), ...);
	return create_request<rq::modify_role>(body.dump(), g);
}

template<typename ... settings>
rq::modify_role shard::modify_role(const guild_role& g, modify_role_settings<settings...> s) {
	nlohmann::json body;
	std::apply([&](auto&&... setting) {
		((body[setting.vname] = std::move(setting.n)), ...);
	}, std::move(s.stuff));
	return create_request<rq::modify_role>(body.dump(), g);
}

template<typename ... settings>//requires sizeof...(settings)>=1
rq::modify_webhook shard::modify_webhook(const webhook& wh, ::modify_webhook_settings<settings...> settings_) {
	return std::apply([&](auto&& ... settings__) {
						  return modify_webhook(wh, std::forward<decltype(settings__)>(settings__)...);
					  }, std::move(settings_.settings)
	);
}

template<typename ... settings>//requires sizeof...(settings)>=1
rq::modify_webhook shard::modify_webhook(const webhook& wh, settings&&... s) {
	nlohmann::json body = {};
	((body[s.vname] = std::move(s.n)), ...);
	return create_request<rq::modify_webhook>(body.dump(), wh);
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
	return create_request<rq::create_group_dm>(body.dump());
}

template<typename...modifiers>
requires(message_modifier<modifiers>&&...)
rq::send_message shard::send_reply(const partial_message& msg, std::string content, modifiers&&... modifers_) {
	nlohmann::json body = {};
	body["content"] = std::move(content);
	(modifers_.modify_message_json(body), ...);

	return create_request<rq::send_message>(body.dump(), msg);;
}

template<typename...modifiers>
requires(message_modifier<modifiers>&&...)
rq::send_message shard::send_reply(const partial_message& msg, std::string content, const embed& embed, modifiers&&... modifers_) {
	nlohmann::json body = {};
	body["content"] = std::move(content);
	(modifers_.modify_message_json(body), ...);
	body["embed"] = embed;

	return create_request<rq::send_message>(body.dump(), msg);;
}
// clang-format on
