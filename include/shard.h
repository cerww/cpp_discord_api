#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
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


using namespace std::string_literals;
using namespace std::chrono_literals;

struct client;

struct voice_connection;

struct shard {
	static constexpr int large_threshold = 51;
	//not in here cuz shard.cpp would be too big to compile without /bigobj on vc ;-;
	friend cerwy::task<void> init_shard(int shardN, shard& t_parent, boost::asio::io_context& ioc, std::string_view gateway);

	using wsClient = rename_later_5;
	explicit shard(int shard_number, client* t_parent, boost::asio::io_context& ioc, std::string_view);

	shard& operator=(const shard&) = delete;
	shard& operator=(shard&&) = delete;
	shard(const shard&) = delete;
	shard(shard&&) = delete;

	~shard() noexcept {
		m_client->close(4000);
	}

	const ref_stable_map<snowflake, text_channel>& text_channels() const noexcept { return m_text_channel_map; }

	const ref_stable_map<snowflake, dm_channel>& dm_channels() const noexcept { return m_dm_channels; }

	const ref_stable_map<snowflake, voice_channel>& voice_channels() const noexcept { return m_voice_channel_map; }

	const ref_stable_map<snowflake, channel_catagory>& channel_catagories() const noexcept { return m_channel_catagory_map; }

	rq::send_message send_message(const text_channel& channel, std::string content);
	rq::send_message send_message(const dm_channel& channel, std::string content);

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

	template<typename rng> requires is_range_of_v<rng, partial_message>
	rq::delete_message_bulk delete_message_bulk(const partial_channel&, rng&&);

	template<typename rng> requires is_range_of_v<rng, snowflake>
	rq::delete_message_bulk delete_message_bulk(const partial_channel&, rng&&);

	rq::delete_message_bulk delete_message_bulk(const partial_channel&, std::vector<snowflake>);
	rq::leave_guild leave_guild(const Guild&);
	rq::add_reaction add_reaction(const partial_message&, const partial_emoji&);
	rq::typing_start typing_start(const partial_channel&);
	rq::delete_channel_permission delete_channel_permissions(const partial_guild_channel&, const permission_overwrite&);

	template<typename rng>
	rq::modify_channel_positions modify_channel_positions(const Guild&, rng&&);

	rq::list_guild_members list_guild_members(const partial_guild&, int n = 1, snowflake after = {});
	rq::edit_channel_permissions edit_channel_permissions(const partial_guild_channel&, const permission_overwrite&);
	rq::create_dm create_dm(const user&);
	
	rq::get_guild_integrations get_guild_integrations(const partial_guild& guild);
	
	rq::create_guild_integration create_guild_integration(const partial_guild& guild,std::string type, snowflake id);
	
	
	template<typename range> requires is_range_of<range,std::string>
	rq::create_group_dm create_group_dm(range&& access_tokens, std::unordered_map<snowflake, std::string> nicks = {});
	
	/// @brief 
	/// @tparam rng 
	/// @param guild 
	/// @param id 
	/// @param access_token 
	/// @param roles 
	/// @param nick 
	/// @param deaf 
	/// @param mute 
	/// @return 
	template<typename rng> requires is_range_of<rng,snowflake>
	rq::add_guild_member add_guild_member(const Guild& guild, snowflake id, std::string access_token, rng&& roles, std::string nick = "", bool deaf = false, bool mute = false);
	
	/// @brief 
	/// @tparam rng 
	/// @param guild 
	/// @param id 
	/// @param access_token 
	/// @param roles 
	/// @param nick 
	/// @param deaf 
	/// @param mute 
	/// @return 
	template<typename rng>requires is_range_of<rng, guild_role>
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
	
	bool is_disconnected() const noexcept {
		return m_is_disconnected;
	}

	void update_presence(Status, std::string);

	const user& self_user() const noexcept {
		return m_self_user;
	}

	std::string_view session_id() const noexcept {
		return m_session_id;
	}

	nlohmann::json presence() const;

	cerwy::task<voice_connection> connect_voice(const voice_channel&);

	boost::asio::io_context::strand& strand() {
		return m_strand;
	}

	auto& resolver() {
		return m_resolver;
	}

	const auto& resolver() const {
		return m_resolver;
	}

	auto& ssl_context() {
		return m_ssl_ctx;
	}

	const auto& ssl_context() const {
		return m_ssl_ctx;
	}

	client& parent_client() {
		return *m_parent;
	}

private:
	cerwy::task<boost::beast::error_code> connect_http_connection();

	void doStuff(nlohmann::json, int);
	void on_reconnect();

	void rate_limit(std::chrono::system_clock::time_point tp);

	void close_connection(int code);

	void reconnect();
	void send_heartbeat();
	void send_resume() const;

	template<typename T, typename ... args>
	std::enable_if_t<rq::has_content_type_v<T>, T> send_request(std::string&&, args&&...);

	template<typename T, typename ... args>
	std::enable_if_t<!rq::has_content_type_v<T>, T> send_request(args&&...);

	bool m_is_disconnected = false;

	void set_up_request(boost::beast::http::request<boost::beast::http::string_body>&) const;

	void request_guild_members(Guild& g) const;

	//dispatch
	void m_opcode0(nlohmann::json, event_name, size_t);
	//heartbeat
	void m_opcode1_send_heartbeat() const;
	//identify
	void m_opcode2_send_identity() const;
	//status update
	void m_opcode3_send_presence() const;//update presence
	//voice state
	std::pair<cerwy::task<nlohmann::json>, cerwy::task<std::string>> m_opcode4(const voice_channel&);
	//resume
	void m_opcode6_send_resume() const;
	//reconnect
	void m_opcode7_reconnect();
	//request guild members
	void m_opcode8_guild_member_chunk(snowflake) const;
	//invalid session
	cerwy::task<void> m_opcode9_on_invalid_session(const nlohmann::json&);
	//hello
	void m_opcode10_on_hello(nlohmann::json&);
	//heartbeat ack
	void m_opcode11(nlohmann::json&);

	void m_sendIdentity() const;

	//event stuff
	template<event_name e>
	void procces_event(nlohmann::json&);
	template<>
	void procces_event<event_name::HELLO>(nlohmann::json&);
	template<>
	void procces_event<event_name::READY>(nlohmann::json&);
	template<>
	void procces_event<event_name::RESUMED>(nlohmann::json&);
	template<>
	void procces_event<event_name::INVALID_SESSION>(nlohmann::json&);
	template<>
	void procces_event<event_name::CHANNEL_CREATE>(nlohmann::json&);
	template<>
	void procces_event<event_name::CHANNEL_DELETE>(nlohmann::json&);
	template<>
	void procces_event<event_name::CHANNEL_UPDATE>(nlohmann::json&);
	template<>
	void procces_event<event_name::CHANNEL_PINS_UPDATE>(nlohmann::json&);
	template<>
	void procces_event<event_name::GUILD_CREATE>(nlohmann::json&);
	template<>
	void procces_event<event_name::GUILD_UPDATE>(nlohmann::json&);
	template<>
	void procces_event<event_name::GUILD_DELETE>(nlohmann::json&);
	template<>
	void procces_event<event_name::GUILD_BAN_ADD>(nlohmann::json&);
	template<>
	void procces_event<event_name::GUILD_BAN_REMOVE>(nlohmann::json&);
	template<>
	void procces_event<event_name::GUILD_EMOJI_UPDATE>(nlohmann::json&);
	template<>
	void procces_event<event_name::GUILD_INTEGRATIONS_UPDATE>(nlohmann::json&);
	template<>
	void procces_event<event_name::GUILD_MEMBER_ADD>(nlohmann::json&);
	template<>
	void procces_event<event_name::GUILD_MEMBER_REMOVE>(nlohmann::json&);
	template<>
	void procces_event<event_name::GUILD_MEMBER_UPDATE>(nlohmann::json&);
	template<>
	void procces_event<event_name::GUILD_MEMBERS_CHUNK>(nlohmann::json&);
	template<>
	void procces_event<event_name::GUILD_ROLE_CREATE>(nlohmann::json&);
	template<>
	void procces_event<event_name::GUILD_ROLE_UPDATE>(nlohmann::json&);
	template<>
	void procces_event<event_name::GUILD_ROLE_DELETE>(nlohmann::json&);
	template<>
	void procces_event<event_name::MESSAGE_CREATE>(nlohmann::json&);
	template<>
	void procces_event<event_name::MESSAGE_UPDATE>(nlohmann::json&);
	template<>
	void procces_event<event_name::MESSAGE_DELETE>(nlohmann::json&);
	template<>
	void procces_event<event_name::MESSAGE_DELETE_BULK>(nlohmann::json&);
	template<>
	void procces_event<event_name::MESSAGE_REACTION_ADD>(nlohmann::json&);
	template<>
	void procces_event<event_name::MESSAGE_REACTION_REMOVE>(nlohmann::json&);
	template<>
	void procces_event<event_name::MESSAGE_REACTION_REMOVE_ALL>(nlohmann::json&);
	template<>
	void procces_event<event_name::PRESENCE_UPDATE>(nlohmann::json&);
	template<>
	void procces_event<event_name::TYPING_START>(nlohmann::json&);
	template<>
	void procces_event<event_name::USER_UPDATE>(nlohmann::json&);
	template<>
	void procces_event<event_name::VOICE_STATE_UPDATE>(nlohmann::json&);
	template<>
	void procces_event<event_name::VOICE_SERVER_UPDATE>(nlohmann::json&);
	template<>
	void procces_event<event_name::WEBHOOKS_UPDATE>(nlohmann::json&);

	template<int n>
	static reaction& update_reactions(std::vector<reaction>&, partial_emoji&, snowflake, snowflake my_id);
	static reaction& add_reaction(std::vector<reaction>&, partial_emoji&, snowflake, snowflake);
	static reaction& remove_reaction(std::vector<reaction>&, partial_emoji&, snowflake, snowflake);

	template<typename msg_t, typename channel_t, typename map_t>
	msg_t create_msg(channel_t&, const nlohmann::json&, map_t&&);

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

	ref_stable_map<snowflake, Guild> m_guilds;
	ref_stable_map<snowflake, text_channel> m_text_channel_map;
	ref_stable_map<snowflake, voice_channel> m_voice_channel_map;
	ref_stable_map<snowflake, channel_catagory> m_channel_catagory_map;
	ref_stable_map<snowflake, dm_channel> m_dm_channels;

	user m_self_user;
	Status m_status = Status::online;
	std::string m_game_name;

	std::string m_session_id;

	size_t m_seqNum = 0;
	std::atomic<bool> m_done = false;

	const int m_shard_number = 0;
	client* m_parent = nullptr;

	std::unique_ptr<wsClient> m_client = nullptr;
	http_connection2 m_http_connection;

	boost::asio::io_context& m_ioc;
	boost::asio::ssl::context m_ssl_ctx{boost::asio::ssl::context_base::sslv23};
	
	boost::asio::ip::tcp::resolver m_resolver;
	boost::asio::io_context::strand m_strand;
	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>> m_socket;

	//events that came before guild recieved all members from guild_member_chunk
	ska::bytell_hash_map<snowflake, std::vector<std::pair<nlohmann::json, event_name>>> m_backed_up_events;
	void replay_events_for(snowflake);
	ska::bytell_hash_map<snowflake, int> m_chunks_left_for_guild;

	//TODO: rename these
	ska::bytell_hash_map<snowflake, cerwy::promise<nlohmann::json>> m_things_waiting_for_voice_endpoint;
	ska::bytell_hash_map<snowflake, cerwy::promise<std::string>> m_things_waiting_for_voice_endpoint2;

	friend struct client;
};

namespace rawrland {//rename later ;-;

	template<typename fut_type, typename ... Args>
	std::pair<fut_type, discord_request> get_default_stuffs(Args&&... args) {
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
	auto [retVal, r] = rawrland::get_default_stuffs<T>(std::forward<args>(Args)...);
	set_up_request(r.req);
	r.req.body() = std::move(body);
	r.req.prepare_payload();
	r.state->strand = &m_strand;
	m_http_connection.send(std::move(r));
	return std::move(retVal);//no nrvo
}

template<typename T, typename ... args>
std::enable_if_t<!rq::has_content_type_v<T>, T> shard::send_request(args&&... Args) {
	auto [retVal, r] = rawrland::get_default_stuffs<T>(std::forward<args>(Args)...);
	set_up_request(r.req);
	r.req.prepare_payload();
	r.state->strand = &m_strand;
	m_http_connection.send(std::move(r));
	return std::move(retVal);//no nrvo
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

template<typename rng> requires is_range_of_v<rng,partial_message>
rq::delete_message_bulk shard::delete_message_bulk(const partial_channel& channel, rng&& msgs) {
	nlohmann::json body;
	body["messages"] = msgs | ranges::views::transform(&partial_message::id) | ranges::to<std::vector>;
	return send_request<rq::delete_message_bulk>(body.dump(), channel);
}

template<typename rng> requires is_range_of_v<rng, snowflake>
rq::delete_message_bulk shard::delete_message_bulk(const partial_channel& channel, rng&& msgs) {
	nlohmann::json body;
	if constexpr (std::is_convertible_v<rng, nlohmann::json>) {
		body["messages"] = msgs;
	}else {
		body["messages"] = msgs | ranges::to<std::vector>;
	}
	return send_request<rq::delete_message_bulk>(body.dump(), channel);
}

template<typename rng> requires is_range_of<rng,snowflake>
rq::add_guild_member shard::add_guild_member(const Guild& guild, snowflake id, std::string access_token, rng&& roles, std::string nick, bool deaf, bool mute) {
	nlohmann::json body;
	body["access_token"] = access_token;
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

template<typename rng> requires is_range_of<rng, guild_role>
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



template<typename range>requires is_range_of<range, std::string>
rq::create_group_dm shard::create_group_dm(range&& access_tokens, std::unordered_map<snowflake, std::string> nicks) {
	nlohmann::json body;
	body["access_tokens"] = std::vector<int>();
	for (auto&& token : access_tokens) {
		body["access_tokens"].push_back(token);
	}
	body["nicks"] = std::move(nicks);
	return send_request<rq::create_group_dm>(body.dump());
}

template<event_name e>
void shard::procces_event(nlohmann::json&) {
	//static_assert(false);
}

template<typename msg_t, typename channel_t, typename map_t>
msg_t shard::create_msg(channel_t& ch, const nlohmann::json& stuffs, map_t&& members_in_channel) {
	msg_t retVal;
	from_json(stuffs, static_cast<partial_message&>(retVal));
	retVal.m_channel = &ch;
	retVal.m_author = &members_in_channel[retVal.author_id()];
	retVal.m_mentions.reserve(stuffs["mentions"].size());
	for (const auto& mention : stuffs["mentions"]) {
		const auto it = members_in_channel.find(mention["id"].get<snowflake>());
		//idk why this would happen
		if (it == members_in_channel.end())
			continue;
		retVal.m_mentions.push_back(&(it->second));
	}

	if constexpr (std::is_same_v<msg_t, guild_text_message>) {
		retVal.m_mention_roles_ids = stuffs["mention_roles"].get<std::vector<snowflake>>();
		const Guild& guild = *ch.m_guild;
		retVal.m_mention_roles.reserve(retVal.m_mention_roles_ids.size());
		for (const auto& role_id : retVal.m_mention_roles_ids)
			retVal.m_mention_roles.push_back(&guild.m_roles.at(role_id));
	}
	return retVal;
}

template<typename msg_t, typename channel_t, typename map_t>
msg_t shard::createMsgUpdate(channel_t& ch, const nlohmann::json& stuffs, map_t&& members) {
	msg_t retVal;
	from_json(stuffs, static_cast<msg_update&>(retVal));
	retVal.m_channel = &ch;
	//retVal.m_author = members[stuffs["author"]["id"]];
	if (retVal.m_author_id.val) {
		retVal.m_author = &members[retVal.m_author_id];
	}
	auto it_to_mentions = stuffs.find("mentions");
	if (it_to_mentions != stuffs.end()) {
		for (const auto& mention : *it_to_mentions)/* *it is list of users*/ {
			auto& t = members[mention["id"].get<snowflake>()];
			retVal.m_mentions.push_back(&t);
		}
	}

	if constexpr (std::is_same_v<msg_t, guild_text_message>)
		retVal.m_mention_roles_ids = stuffs.value("mention_roles", std::vector<snowflake>());

	return retVal;
}

inline reaction& shard::add_reaction(std::vector<reaction>& a, partial_emoji& b, snowflake c, snowflake d) {
	return update_reactions<1>(a, b, c, d);
}

inline reaction& shard::remove_reaction(std::vector<reaction>& a, partial_emoji& b, snowflake c, snowflake d) {
	return update_reactions<-1>(a, b, c, d);
}

//this function was hacky ;-;
template<int n>
reaction& shard::update_reactions(
	std::vector<reaction>& reactions,
	partial_emoji& emoji,
	const snowflake user_id,
	const snowflake my_id
) {
	static_assert(n == -1 || n == 1);
	const auto it = ranges::find(reactions, emoji.id(), hof::flow(&reaction::emoji, &emoji::id));
	if (it == reactions.end()) {//reaction not in vector -> add it 
		reaction temp;
		temp.m_count = n;
		temp.m_emoji = std::move(emoji);
		temp.m_me = user_id == my_id;
		return reactions.emplace_back(std::move(temp));
	}
	else {
		reaction& r = *it;
		r.m_count += n;

		if (my_id == user_id) {
			r.m_me = n > 0;//true if n == 1, false if n==-1
		}
		return r;
	}
}
