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
#include "intents.h"
#include "shard.h"


using namespace std::string_literals;
using namespace std::chrono_literals;

struct client;

struct voice_connection;

<<<<<<< HEAD
struct internal_shard :shard {
=======
struct internal_shard: shard {
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
	static constexpr int large_threshold = 51;

	
	//not in here cuz shard.cpp would be too big to compile without /bigobj on vc ;-;
	friend cerwy::task<void> init_shard(int shardN, internal_shard& t_parent, boost::asio::io_context& ioc, std::string_view gateway);

	using wsClient = rename_later_5;
	explicit internal_shard(int shard_number, client* t_parent, boost::asio::io_context& ioc, std::string_view, intents);

	internal_shard& operator=(const internal_shard&) = delete;
	internal_shard& operator=(internal_shard&&) = delete;
	internal_shard(const internal_shard&) = delete;
	internal_shard(internal_shard&&) = delete;

	~internal_shard() noexcept {
		m_client->close(4000);
	}

<<<<<<< HEAD

=======
	const ref_stable_map<snowflake, text_channel>& text_channels() const noexcept { return m_text_channel_map; }

	const ref_stable_map<snowflake, dm_channel>& dm_channels() const noexcept { return m_dm_channels; }

	const ref_stable_map<snowflake, voice_channel>& voice_channels() const noexcept { return m_voice_channel_map; }

	const ref_stable_map<snowflake, channel_catagory>& channel_catagories() const noexcept { return m_channel_catagory_map; }

	
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
	bool is_disconnected() const noexcept {
		return m_is_disconnected;
	}

	void update_presence(Status, std::string);

<<<<<<< HEAD

=======
	
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
	std::string_view session_id() const noexcept {
		return m_session_id;
	}

	nlohmann::json presence() const;

	cerwy::task<voice_connection> connect_voice(const voice_channel&);

<<<<<<< HEAD
=======
	
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de

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

<<<<<<< HEAD

	bool m_is_disconnected = false;

=======
	

	bool m_is_disconnected = false;

	
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de

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

<<<<<<< HEAD
	guild_member make_member_from_msg(const nlohmann::json& user_json, const nlohmann::json& member_json);


=======
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
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

<<<<<<< HEAD

=======
	ref_stable_map<snowflake, Guild> m_guilds;
	ref_stable_map<snowflake, text_channel> m_text_channel_map;
	ref_stable_map<snowflake, voice_channel> m_voice_channel_map;
	ref_stable_map<snowflake, channel_catagory> m_channel_catagory_map;
	ref_stable_map<snowflake, dm_channel> m_dm_channels;

	
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
	Status m_status = Status::online;
	std::string m_game_name;

	std::string m_session_id;

	size_t m_seqNum = 0;
	std::atomic<bool> m_done = false;


<<<<<<< HEAD
	boost::asio::io_context& m_ioc;
	boost::asio::ssl::context m_ssl_ctx{boost::asio::ssl::context_base::sslv23};

=======

	boost::asio::io_context& m_ioc;
	boost::asio::ssl::context m_ssl_ctx{boost::asio::ssl::context_base::sslv23};
	
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
	boost::asio::ip::tcp::resolver m_resolver;
	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>> m_socket;

	//events that came before guild recieved all members from guild_member_chunk
	ska::bytell_hash_map<snowflake, std::vector<std::pair<nlohmann::json, event_name>>> m_backed_up_events;
	void replay_events_for(snowflake);
	ska::bytell_hash_map<snowflake, int> m_chunks_left_for_guild;

	//TODO: rename these
	ska::bytell_hash_map<snowflake, cerwy::promise<nlohmann::json>> m_things_waiting_for_voice_endpoint;
	ska::bytell_hash_map<snowflake, cerwy::promise<std::string>> m_things_waiting_for_voice_endpoint2;

	intents m_intents = {};
<<<<<<< HEAD

=======
	
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
	friend struct client;
};


<<<<<<< HEAD
=======

>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
template<event_name e>
void internal_shard::procces_event(nlohmann::json&) {
	//static_assert(false);
}

<<<<<<< HEAD
inline guild_member internal_shard::make_member_from_msg(const nlohmann::json& user_json, const nlohmann::json& member_json) {
	guild_member ret;
	from_json(user_json, (user&)ret);
	const auto it = member_json.find("nick");
	if (it != member_json.end())
		ret.m_nick = member_json["nick"].is_null() ? "" : member_json["nick"].get<std::string>();
	//out.m_roles = in["roles"].get<std::vector<snowflake>>();

	ret.m_roles = member_json["roles"] | ranges::views::transform(&nlohmann::json::get<snowflake>) | ranges::to<boost::container::small_vector<snowflake, 5>>();

	//out.m_joined_at = in["joined_at"].get<timestamp>();

	ret.m_deaf = member_json["deaf"].get<bool>();
	ret.m_mute = member_json["mute"].get<bool>();

	return ret;
}

=======
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
template<typename msg_t, typename channel_t, typename map_t>
msg_t internal_shard::create_msg(channel_t& ch, const nlohmann::json& stuffs, map_t&& members_in_channel) {
	msg_t retVal;
	from_json(stuffs, static_cast<partial_message&>(retVal));
	retVal.m_channel = &ch;
<<<<<<< HEAD
	//retVal.m_author = &members_in_channel[retVal.author_id()];
	//retVal.m_mentions.reserve(stuffs["mentions"].size());
	/*
=======
	retVal.m_author = &members_in_channel[retVal.author_id()];
	retVal.m_mentions.reserve(stuffs["mentions"].size());
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
	for (const auto& mention : stuffs["mentions"]) {
		const auto it = members_in_channel.find(mention["id"].get<snowflake>());
		//idk why this would happen
		if (it == members_in_channel.end())
			continue;
		retVal.m_mentions.push_back(&(it->second));
	}
<<<<<<< HEAD
	*/
	constexpr bool is_guild_msg = std::is_same_v<msg_t, guild_text_message>;

	if constexpr (is_guild_msg) {
		retVal.m_author = make_member_from_msg(stuffs["author"], stuffs["member"]);
		retVal.m_author.m_guild = ch.m_guild;
	} else {
		retVal.m_author = stuffs["author"].get<user>();
	}


	if constexpr (is_guild_msg) {
		retVal.m_mentions.reserve(stuffs["mentions"].size());
		for (const auto& mention : stuffs["mentions"]) {
			auto& member = retVal.m_mentions.emplace_back(make_member_from_msg(mention, mention["member"]));
			member.m_guild = ch.m_guild;
		}
	}else {
		retVal.m_mentions = stuffs["mentions"].get<std::vector<user>>();
	}

=======
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de

	if constexpr (std::is_same_v<msg_t, guild_text_message>) {
		retVal.m_mention_roles_ids = stuffs["mention_roles"].get<std::vector<snowflake>>();
		const Guild& guild = *ch.m_guild;
		retVal.m_mention_roles.reserve(retVal.m_mention_roles_ids.size());
		for (const auto& role_id : retVal.m_mention_roles_ids)
			retVal.m_mention_roles.push_back(&guild.m_roles.at(role_id));
	}
<<<<<<< HEAD

	
	
=======
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
	return retVal;
}

template<typename msg_t, typename channel_t, typename map_t>
msg_t internal_shard::createMsgUpdate(channel_t& ch, const nlohmann::json& stuffs, map_t&& members) {
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

inline reaction& internal_shard::add_reaction(std::vector<reaction>& a, partial_emoji& b, snowflake c, snowflake d) {
	return update_reactions<1>(a, b, c, d);
}

inline reaction& internal_shard::remove_reaction(std::vector<reaction>& a, partial_emoji& b, snowflake c, snowflake d) {
	return update_reactions<-1>(a, b, c, d);
}

//this function was hacky ;-;
template<int n>//n = 1 means add_reaction, -1 means remove
reaction& internal_shard::update_reactions(
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
<<<<<<< HEAD
	} else {
=======
	}
	else {
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
		reaction& r = *it;
		r.m_count += n;

		if (my_id == user_id) {
			r.m_me = n > 0;//true if n == 1, false if n==-1
		}
		return r;
	}
}
