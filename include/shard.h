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
	shard(int shardN, wsClient* t_client, client* t_parent) :
		m_shard_number(shardN),
		m_parent(t_parent), 
		m_client(t_client),
		m_name_plox(t_parent)	{
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
		if (m_heartbeatThread.joinable())
			m_heartbeatThread.join();
		if (m_main_thread.joinable())
			m_main_thread.join();
	}
	std::unordered_map<snowflake, text_channel> & text_channels() noexcept { return m_text_channels; }
	const std::unordered_map<snowflake, text_channel> & text_channels() const noexcept { return m_text_channels; }

	std::unordered_map<snowflake, dm_channel>& dm_channels()noexcept { return m_dm_channels; }
	const std::unordered_map<snowflake, dm_channel>& dm_channels()const noexcept { return m_dm_channels; }

	std::unordered_map<snowflake, voice_channel>& voice_channels() noexcept { return m_voice_channels; }
	const std::unordered_map<snowflake, voice_channel>& voice_channels() const noexcept { return m_voice_channels; }

	std::unordered_map<snowflake, channel_catagory>& channel_catagories()noexcept { return m_channel_catagories; }
	const std::unordered_map<snowflake, channel_catagory>& channel_catagories()const noexcept { return m_channel_catagories; }

	rq::send_message send_message(const text_channel& channel, std::string content);
	rq::send_message send_message(const dm_channel& channel, std::string content);

	rq::add_role add_role(const Guild&, const guild_member&, const Role&);
	rq::remove_role remove_role(const Guild&, const  guild_member&, const Role&);
	rq::modify_member remove_all_roles(const Guild&, const guild_member&);

	rq::create_role create_role(const Guild&, std::string_view, permission, int color = 0xffffff/*white*/, bool hoist = true, bool mentionable = true);
	rq::delete_role delete_role(const Guild&, const Role&);

	rq::modify_member change_nick(const guild_member&, std::string);
	rq::modify_member change_nick(const Guild&, const User&, std::string);
	rq::change_my_nick change_my_nick(const Guild&, std::string_view);
	rq::kick_member kick_member(const Guild&, const  guild_member&);
	rq::ban_member ban_member(const Guild& guild, const guild_member& member, std::string reason = "", int days_to_delete_msg = 0);
	rq::unban_member unban_member(const Guild&, snowflake id);
	rq::get_messages get_messages(const Channel&,int = 100);
	rq::get_messages get_messages_before(const Channel&,snowflake, int = 100);
	rq::get_messages get_messages_after(const Channel&, snowflake, int = 100);
	rq::get_messages get_messages_around(const Channel&, snowflake, int = 100);
	rq::get_messages get_messages_before(const Channel&, const partial_message&, int = 100);
	rq::get_messages get_messages_after(const Channel&, const partial_message&, int = 100);
	rq::get_messages get_messages_around(const Channel&, const partial_message&, int = 100);
	rq::create_text_channel create_text_channel(const Guild&, std::string,std::vector<permission_overwrite> = {},bool = false);
	rq::edit_message edit_message(const partial_message&,std::string);
	rq::create_voice_channel create_voice_channel(const Guild&, std::string, std::vector<permission_overwrite> = {}, bool = false, int = 96);
	rq::create_channel_catagory create_channel_catagory(const Guild&, std::string, std::vector<permission_overwrite> = {}, bool = false);
	rq::delete_emoji delete_emoji(const Guild&, const emoji&);
	//Awaitables::modify_emoji modify_emoji(Guild&, emoji&, std::string, std::vector<snowflake>);
	rq::delete_message delete_message(const partial_message&);
	rq::delete_message_bulk delete_message_bulk(const Channel&, const std::vector<partial_message>&);
	rq::delete_message_bulk delete_message_bulk(const Channel&,std::vector<snowflake>);
	rq::leave_guild leave_guild(const Guild&);
	rq::add_reaction add_reaction(const partial_message&, const partial_emoji&);
	rq::typing_start typing_start(const Channel&);
	rq::delete_channel_permission delete_channel_permissions(const guild_channel&, const permission_overwrite&);
	//Awaitables::modfiy_guild 
	rq::modify_channel_positions modify_channel_positions(const Guild&, const std::vector<std::pair<snowflake,int>>&);

	template<typename rng>
	std::enable_if_t<std::is_same_v<std::decay_t<range_type<rng>>, snowflake>, rq::add_guild_member> add_guild_member(const Guild&,snowflake,std::string,rng&&,std::string = "",bool = false,bool = false);

	template<typename rng>
	std::enable_if_t<std::is_same_v<std::decay_t<range_type<rng>>, Role>, rq::add_guild_member> add_guild_member(const Guild&, snowflake, std::string, rng&&, std::string = "", bool = false, bool = false);

	rq::delete_channel delete_channel(const Channel&);
	rq::add_pinned_msg add_pinned_msg(const Channel&,const partial_message&);
	rq::remove_pinned_msg remove_pinned_msg(const Channel&, const partial_message&);
	rq::get_voice_regions get_voice_regions();

	bool is_disconnected() const {
		return m_is_disconnected;
	}
	void update_presence(const Status,std::string);

private:
	void add_event(std::string t) {
		m_event_queue.push(std::move(t));
	};
	void doStuff(nlohmann::json);
	void rate_limit(std::chrono::steady_clock::time_point tp) {
		m_name_plox.sleep_till(tp);
	}

	void reconnect2() {
		m_is_disconnected = true;
		m_client->close();
	}

	void reconnect();
	void send_resume() const;

	template<typename T,typename ... args>	
	std::enable_if_t<rq::has_content_type_v<T>,T> m_send_things(std::string&&, args&&...);

	template<typename T, typename ... args>
	std::enable_if_t<!rq::has_content_type_v<T>, T> m_send_things(args&&...);
	bool m_is_disconnected = false;
	
	void set_up_request(boost::beast::http::request<boost::beast::http::string_body>&)const;
	const int m_shard_number = 0;
	client* m_parent = nullptr;

	uWS::WebSocket<uWS::CLIENT>* m_client = nullptr;
	discord_http_connection m_name_plox;

	std::thread m_main_thread;
	concurrent_queue<std::string,std::vector<std::string>> m_event_queue;

	void m_opcode0(nlohmann::json, eventName, size_t);
	void m_opcode1() const;
	void m_opcode2() const;
	void m_opcode3() const;//update presence
	void m_opcode6() const;
	void m_opcode7();

	void m_opcode8() const;
	void m_opcode9(const nlohmann::json&) const;
	void m_opcode10(nlohmann::json&);
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
	
	template<typename msg_t,typename channel_t,typename map_t>
	msg_t createMsg(channel_t&, const nlohmann::json&,map_t&&);

	template<typename msg_t, typename channel_t, typename map_t>
	msg_t createMsgUpdate(channel_t&, const nlohmann::json&, map_t&&);
	//HB stuff
	std::atomic<bool> m_op11 = true;
	size_t m_HBd = 0;
	std::thread m_heartbeatThread;
	int m_hb_interval = 0;
	//trace stuff
	nlohmann::json m_trace;//idk what this is;-;
	nlohmann::json m_trace2;//;-;

	//discord object stuffs

	std::unordered_map<snowflake, Guild> m_guilds;
	std::unordered_map<snowflake, text_channel> m_text_channels;
	std::unordered_map<snowflake, voice_channel> m_voice_channels;
	std::unordered_map<snowflake, channel_catagory> m_channel_catagories;
	//std::unordered_map<snowflake,voi
	//std::vector<DMChannel> m_dm_channels;
	std::unordered_map<snowflake, dm_channel> m_dm_channels;
	std::string session_id;
	size_t m_seqNum = 0;
	std::atomic<bool> m_done = false;
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
	r.req.body() = body;
	r.req.prepare_payload();
	m_name_plox.add(std::move(r));
	return std::move(retVal);
}

template <typename T, typename ... args>
std::enable_if_t<!rq::has_content_type_v<T>, T> shard::m_send_things(args&&... Args) {
	auto[retVal, r] = rawrland::get_default_stuffs<T>(std::forward<args>(Args)...);
	set_up_request(r.req);
	r.req.prepare_payload();
	m_name_plox.add(std::move(r));
	return std::move(retVal);
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
std::enable_if_t<std::is_same_v<std::decay_t<range_type<rng>>, Role>, rq::add_guild_member> shard::add_guild_member(const Guild& guild , snowflake id, std::string access_token, rng&& roles, std::string nick, bool deaf, bool mute) {
	nlohmann::json body;
	body["access_token"] = access_token;
	body["nick"] = std::move(nick);		
	body["deaf"] = deaf;
	body["mute"] = mute;	
	body["roles"] = std::vector<int>();
	std::transform(roles.begin(), roles.end(), body["roles"].begin(), [](auto&& role) {return role.id(); });	
	return m_send_things<rq::add_guild_member>(body.dump(), guild, id);
}

template <eventName e>
void shard::m_procces_event(const nlohmann::json&) {}

template <typename msg_t, typename channel_t,typename map_t>
msg_t shard::createMsg(channel_t& ch, const nlohmann::json& stuffs,map_t&& members) {
	msg_t retVal;
	from_json(stuffs, static_cast<partial_message&>(retVal));
	retVal.m_channel = &ch;
	retVal.m_author = members[retVal.author_id()];
	for (const auto& mention : stuffs["mentions"])
		retVal.m_mentions.push_back(members[mention["id"].get<snowflake>()]);

	if constexpr(std::is_same_v<msg_t,guild_text_message>)
		retVal.m_mention_roles = stuffs["mention_roles"].get<std::vector<snowflake>>();
	
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
		retVal.m_mention_roles = stuffs.value("mention_roles", std::vector<snowflake>());

	return retVal;
}

