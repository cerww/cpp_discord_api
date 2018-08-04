#pragma once
#include <uWS/uWS.h>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>
#include "constant_stuffs.h"
#include "snowflake.h"
#include "guild.h"
#include "dm_channel.h"
#include "voice_channel.h"
#include "channel_catagory.h"
#include "partial_message.h"
#include "Awaitables.h"
#include "discord_http_connection.h"
#include <type_traits>
#include "range-like-stuffs.h"

using namespace std::string_literals;
using namespace std::chrono_literals;
class client;

class shard {
public:
	using wsClient = uWS::WebSocket<uWS::CLIENT>;
	explicit shard(int shardN, wsClient* t_client, client* t_parent) :
		m_shard_number(shardN),
		m_parent(t_parent), 
		m_client(t_client),
		m_http_connection(t_parent)	{
		m_main_thread = std::thread([&]() {	
			while (!m_done) {
				doStuff(nlohmann::json::parse(m_event_queue.pop()));
			}
		});
	}
	
	shard& operator=(const shard&) = delete;
	shard& operator=(shard&&) = delete;
	shard(const shard&) = delete;
	shard(shard&&) = delete;
	
	~shard() {
		m_done = true;
		if (m_main_thread.joinable())
			m_main_thread.join();
	}
	
	rename_later_4<snowflake, text_channel> & text_channels() noexcept { return m_text_channels; }
	const rename_later_4<snowflake, text_channel> & text_channels() const noexcept { return m_text_channels; }

	rename_later_4<snowflake, dm_channel>& dm_channels()noexcept { return m_dm_channels; }
	const rename_later_4<snowflake, dm_channel>& dm_channels()const noexcept { return m_dm_channels; }

	rename_later_4<snowflake, voice_channel>& voice_channels() noexcept { return m_voice_channels; }
	const rename_later_4<snowflake, voice_channel>& voice_channels() const noexcept { return m_voice_channels; }

	rename_later_4<snowflake, channel_catagory>& channel_catagories()noexcept { return m_channel_catagories; }
	const rename_later_4<snowflake, channel_catagory>& channel_catagories()const noexcept { return m_channel_catagories; }
	
	rq::send_message send_message(const text_channel& channel, std::string content);
	rq::send_message send_message(const dm_channel& channel, std::string content);

	rq::add_role add_role(const partial_guild&, const guild_member&, const guild_role&);
	rq::remove_role remove_role(const partial_guild&, const  guild_member&, const guild_role&);
	rq::modify_member remove_all_roles(const partial_guild&, const guild_member&);

	rq::create_role create_role(const partial_guild&, std::string_view, permission, int color = 0xffffff/*white*/, bool hoist = true, bool mentionable = true);
	rq::delete_role delete_role(const partial_guild&, const guild_role&);

	rq::modify_member change_nick(const guild_member&, std::string);
	rq::modify_member change_nick(const partial_guild&, const user&, std::string);
	rq::change_my_nick change_my_nick(const partial_guild&, std::string_view);
	rq::kick_member kick_member(const partial_guild&, const  guild_member&);
	rq::ban_member ban_member(const partial_guild& g, const guild_member& member, std::string reason = "", int days_to_delete_msg = 0);
	rq::unban_member unban_member(const Guild&, snowflake id);
	rq::get_messages get_messages(const partial_channel&,int = 100);
	rq::get_messages get_messages_before(const partial_channel&,snowflake, int = 100);
	rq::get_messages get_messages_after(const partial_channel&, snowflake, int = 100);
	rq::get_messages get_messages_around(const partial_channel&, snowflake, int = 100);
	rq::get_messages get_messages_before(const partial_channel&, const partial_message&, int = 100);
	rq::get_messages get_messages_after(const partial_channel&, const partial_message&, int = 100);
	rq::get_messages get_messages_around(const partial_channel&, const partial_message&, int = 100);
	rq::create_text_channel create_text_channel(const Guild&, std::string,std::vector<permission_overwrite> = {},bool = false);
	rq::edit_message edit_message(const partial_message&,std::string);
	rq::create_voice_channel create_voice_channel(const Guild&, std::string, std::vector<permission_overwrite> = {}, bool = false, int = 96);
	rq::create_channel_catagory create_channel_catagory(const Guild&, std::string, std::vector<permission_overwrite> = {}, bool = false);
	rq::delete_emoji delete_emoji(const partial_guild&, const emoji&);
	//Awaitables::modify_emoji modify_emoji(Guild&, emoji&, std::string, std::vector<snowflake>);
	rq::delete_message delete_message(const partial_message&);
	rq::delete_message_bulk delete_message_bulk(const partial_channel&, const std::vector<partial_message>&);
	rq::delete_message_bulk delete_message_bulk(const partial_channel&,std::vector<snowflake>);
	rq::leave_guild leave_guild(const Guild&);
	rq::add_reaction add_reaction(const partial_message&, const partial_emoji&);
	rq::typing_start typing_start(const partial_channel&);
	rq::delete_channel_permission delete_channel_permissions(const guild_channel&, const permission_overwrite&);
	//Awaitables::modfiy_guild 
	rq::modify_channel_positions modify_channel_positions(const Guild&, const std::vector<std::pair<snowflake,int>>&);
	rq::list_guild_members list_guild_members(const partial_guild&,int n = 1,snowflake after = {});
	rq::edit_channel_permissions edit_channel_permissions(const guild_channel&, const permission_overwrite&);
	rq::create_dm create_dm(const user&);
	template<typename range>
	std::enable_if_t<is_range_of<range,std::string>::value,rq::create_group_dm> create_group_dm(range&& access_tokens,std::unordered_map<snowflake,std::string> nicks = {});

	template<typename rng>
	std::enable_if_t<std::is_same_v<std::decay_t<range_type<rng>>, snowflake>, rq::add_guild_member> add_guild_member(const Guild&,snowflake,std::string,rng&&,std::string = "",bool = false,bool = false);

	template<typename rng>
	std::enable_if_t<std::is_same_v<std::decay_t<range_type<rng>>, guild_role>, rq::add_guild_member> add_guild_member(const Guild&, snowflake, std::string, rng&&, std::string = "", bool = false, bool = false);

	rq::delete_channel delete_channel(const partial_channel&);
	rq::add_pinned_msg add_pinned_msg(const partial_channel&,const partial_message&);
	rq::remove_pinned_msg remove_pinned_msg(const partial_channel&, const partial_message&);
	rq::get_voice_regions get_voice_regions();
	rq::create_channel_invite create_channel_invite(const guild_channel&, int max_age = 86400, int max_uses = 0, bool temporary = false, bool unique = false);




	bool is_disconnected() const {
		return m_is_disconnected;
	}
	void update_presence(Status,std::string);

private:
	void add_event(std::string t) {
		m_event_queue.push(std::move(t));
	};
	void doStuff(nlohmann::json);
	void rate_limit(std::chrono::system_clock::time_point tp) {
		m_http_connection.sleep_till(tp);
	}

	void close_connection(int code) {
		m_is_disconnected = true;
		m_client->close(code);
	}

	void reconnect();	
	void send_heartbeat();
	void send_resume() const;

	template<typename T,typename ... args>	
	std::enable_if_t<rq::has_content_type_v<T>,T> m_send_things(std::string&&, args&&...);

	template<typename T, typename ... args>
	std::enable_if_t<!rq::has_content_type_v<T>, T> m_send_things(args&&...);
	bool m_is_disconnected = false;
	
	void set_up_request(boost::beast::http::request<boost::beast::http::string_body>&)const;


	void request_guild_members(Guild& g)const {
		m_opcode8(g.id());
		g.m_members.clear();
		g.m_members.reserve(g.m_member_count);
	}

	//dispatch
	void m_opcode0(nlohmann::json, eventName, size_t);
	//heartbeat
	void m_opcode1() const;
	//identify
	void m_opcode2() const;
	//status update
	void m_opcode3() const;//update presence
	//resume
	void m_opcode6() const;
	//reconnect
	void m_opcode7();
	//request guild members
	void m_opcode8(snowflake) const;
	//invalid session
	void m_opcode9(const nlohmann::json&) const;
	//hello
	void m_opcode10(nlohmann::json&);
	//heartbeat ack
	void m_opcode11(nlohmann::json&);

	void m_sendIdentity()const;

	//event stuff
	template<eventName e>
	void m_procces_event(const nlohmann::json&);
	template<>	void m_procces_event<eventName::HELLO>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::READY>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::RESUMED>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::INVALID_SESSION>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::CHANNEL_CREATE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::CHANNEL_DELETE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::CHANNEL_UPDATE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::CHANNEL_PINS_UPDATE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::GUILD_CREATE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::GUILD_UPDATE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::GUILD_DELETE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::GUILD_BAN_ADD>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::GUILD_BAN_REMOVE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::GUILD_EMOJI_UPDATE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::GUILD_INTEGRATIONS_UPDATE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::GUILD_MEMBER_ADD>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::GUILD_MEMBER_REMOVE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::GUILD_MEMBER_UPDATE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::GUILD_MEMBER_CHUNK>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::GUILD_ROLE_CREATE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::GUILD_ROLE_UPDATE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::GUILD_ROLE_DELETE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::MESSAGE_CREATE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::MESSAGE_UPDATE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::MESSAGE_DELETE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::MESSAGE_DELETE_BULK>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::MESSAGE_REACTION_ADD>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::MESSAGE_REACTION_REMOVE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::MESSAGE_REACTION_REMOVE_ALL>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::PRESENCE_UPDATE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::TYPING_START>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::USER_UPDATE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::VOICE_STATE_UPDATE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::VOICE_SERVER_UPDATE>(const nlohmann::json&);
	template<>	void m_procces_event<eventName::WEBHOOKS_UPDATE>(const nlohmann::json&);

	template<int n>
	static reaction& update_reactions(std::vector<reaction>&, partial_emoji&, snowflake, snowflake my_id);


	template<typename msg_t,typename channel_t,typename map_t>
	msg_t createMsg(channel_t&, const nlohmann::json&,map_t&&);

	template<typename msg_t, typename channel_t, typename map_t>
	msg_t createMsgUpdate(channel_t&, const nlohmann::json&, map_t&&);
	//HB stuff
	std::atomic<bool> m_op11 = true;
	size_t m_HBd = 0;
	int m_hb_interval = 0;
	//trace stuff
	nlohmann::json m_trace;//idk what this is;-;
	nlohmann::json m_trace2;//;-;

	//discord object stuffs

	rename_later_4<snowflake, Guild> m_guilds;
	rename_later_4<snowflake, text_channel> m_text_channels;
	rename_later_4<snowflake, voice_channel> m_voice_channels;
	rename_later_4<snowflake, channel_catagory> m_channel_catagories;
	rename_later_4<snowflake, dm_channel> m_dm_channels;

	std::string session_id;
	size_t m_seqNum = 0;
	std::atomic<bool> m_done = false;

	const int m_shard_number = 0;
	client* m_parent = nullptr;

	uWS::WebSocket<uWS::CLIENT>* m_client = nullptr;
	discord_http_connection m_http_connection;

	std::thread m_main_thread;
	concurrent_queue<std::string, std::vector<std::string>> m_event_queue;

	friend class client;
};

namespace rawrland{//rename later ;-;
template<typename thingy, typename ... Args>
std::pair<thingy, discord_request> get_default_stuffs(Args&&... args) {
	std::pair<thingy, discord_request> retVal;
	retVal.first.state = make_ref_count_ptr<rq::shared_state>();
	retVal.second.state = retVal.first.state;
	retVal.second.req = thingy::request(std::forward<Args>(args)...);
	if constexpr(rq::has_content_type_v<thingy>) {
		retVal.second.req.set(boost::beast::http::field::content_type, thingy::content_type);
	}
	return retVal;
}
}

template <typename T, typename ... args>
std::enable_if_t<rq::has_content_type_v<T>, T> shard::m_send_things(std::string&& body, args&&... Args) {
	auto[retVal, r] = rawrland::get_default_stuffs<T>(std::forward<args>(Args)...);
	set_up_request(r.req);
	r.req.body() = std::move(body);
	r.req.prepare_payload();
	m_http_connection.add(std::move(r));
	return std::move(retVal);//no nrvo
}

template <typename T, typename ... args>
std::enable_if_t<!rq::has_content_type_v<T>, T> shard::m_send_things(args&&... Args) {
	auto[retVal, r] = rawrland::get_default_stuffs<T>(std::forward<args>(Args)...);
	set_up_request(r.req);
	r.req.prepare_payload();
	m_http_connection.add(std::move(r));
	return std::move(retVal);//no nrvo
}

template <typename rng>
std::enable_if_t<std::is_same_v<std::decay_t<range_type<rng>>, snowflake>, rq::add_guild_member> shard::add_guild_member(const Guild& guild, snowflake id, std::string access_token, rng&& roles, std::string nick, bool deaf, bool mute) {
	nlohmann::json body;
	body["access_token"] = access_token;
	body["nick"] = std::move(nick);
	body["deaf"] = deaf;
	body["mute"] = mute;
	if constexpr(std::is_convertible_v<rng,nlohmann::json>)
		body["roles"] = roles;
	else {
		std::vector<snowflake> stuff;
		std::copy(roles.begin(), roles.end(), std::back_inserter(stuff));
		body["roles"] = std::move(stuff);
	}
	return m_send_things<rq::add_guild_member>(body.dump(), guild, id);
}

template <typename rng>
std::enable_if_t<std::is_same_v<std::decay_t<range_type<rng>>, guild_role>, rq::add_guild_member> shard::add_guild_member(const Guild& guild , snowflake id, std::string access_token, rng&& roles, std::string nick, bool deaf, bool mute) {
	nlohmann::json body;
	body["access_token"] = access_token;
	body["nick"] = std::move(nick);		
	body["deaf"] = deaf;
	body["mute"] = mute;	
	body["roles"] = std::vector<int>();
	std::transform(roles.begin(), roles.end(), body["roles"].begin(), [](auto&& role) {return role.id(); });	
	return m_send_things<rq::add_guild_member>(body.dump(), guild, id);
}

template <typename range>
std::enable_if_t<is_range_of<range, std::string>::value, rq::create_group_dm> shard::create_group_dm(range&& access_tokens, std::unordered_map<snowflake, std::string> nicks) {
	nlohmann::json body;
	body["access_tokens"] = std::vector<int>();
	for(auto&& token:access_tokens) {
		body["access_tokens"].push_back(token);
	}
	body["nicks"] = std::move(nicks);
	return m_send_things<rq::create_group_dm>(body.dump());
}


template <eventName e>
void shard::m_procces_event(const nlohmann::json&) {}

template <typename msg_t, typename channel_t,typename map_t>
msg_t shard::createMsg(channel_t& ch, const nlohmann::json& stuffs,map_t&& members) {
	msg_t retVal;
	from_json(stuffs, static_cast<partial_message&>(retVal));
	retVal.m_channel = &ch;
	retVal.m_author = members[retVal.author_id()];
	retVal.m_mentions.reserve(stuffs["mentions"].size());
	for (const auto& mention : stuffs["mentions"])
		retVal.m_mentions.push_back(members[mention["id"].get<snowflake>()]);

	if constexpr(std::is_same_v<msg_t,guild_text_message>){
		retVal.m_mention_roles_ids = stuffs["mention_roles"].get<std::vector<snowflake>>();
		const Guild& guild = *ch.m_guild;
		retVal.m_mention_roles.reserve(retVal.m_mention_roles_ids.size());
		for(const auto& role_id:retVal.m_mention_roles_ids) 
			retVal.m_mention_roles.push_back(&guild.m_roles.at(role_id));				
	}
	return retVal;
}

template <typename msg_t, typename channel_t, typename map_t>
msg_t shard::createMsgUpdate(channel_t& ch, const nlohmann::json& stuffs, map_t&& members) {
	msg_t retVal;
	from_json(stuffs, static_cast<msg_update&>(retVal));
	retVal.m_channel = &ch;
	//retVal.m_author = members[stuffs["author"]["id"]];
	if(retVal.m_author_id.val) {
		retVal.m_author = members[retVal.m_author_id];
	}
	auto it = stuffs.find("mentions");
	if(it != stuffs.end())
		for (const auto& mention : *it)//*it is list of users
			retVal.m_mentions.push_back(members[mention["id"].get<snowflake>()]);

	if constexpr(std::is_same_v<msg_t, guild_text_message>)
		retVal.m_mention_roles_ids = stuffs.value("mention_roles", std::vector<snowflake>());

	return retVal;
}

constexpr bool is_positive(int n) {
	return n > 0;
}

template<int n>
reaction& shard::update_reactions(std::vector<reaction>& reactions, partial_emoji& emoji, snowflake user_id, snowflake my_id) {
	static_assert(n == -1 || n == 1);
	const auto it = std::find_if(reactions.begin(), reactions.end(), [&](const reaction& r) {
		return r.emoji().id() == emoji.id();
	});
	if (it == reactions.end()) {
		reaction temp;
		temp.m_count = n;
		temp.m_emoji = std::move(emoji);
		temp.m_me = user_id == my_id;
		return reactions.emplace_back(std::move(temp));
	}else {
		reaction& r = *it;
		r.m_count += n;

		if(my_id == user_id) {
			r.m_me = n > 0;//true if n == 1, false if n==-1
		}
		return r;
	}
}

