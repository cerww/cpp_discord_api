#include "internal_shard.h"
#include "client.h"
#include "voice_channel.h"
#include "dm_channel.h"
#include "init_shard.h"
#include "voice_connect_impl.h"
#include "../common/executor_binder.h"
#include "unavailable_guild.h"
#include "presence_update.h"
#include "events.h"
#include <concepts>
//#include <boost/beast/>

//copy paste start
#if defined(_WIN32) || defined(_WIN64)
static constexpr const char* s_os = "Windows";
#elif defined(__APPLE__) || defined(__MACH__)
static constexpr const char * s_os = "macOS";
#elif defined(__linux__) || defined(linux) || defined(__linux)
static constexpr const char * s_os = "Linux";
#elif defined __FreeBSD__
static constexpr const char * s_os = "FreeBSD";
#elif defined(unix) || defined(__unix__) || defined(__unix)
static constexpr const char * s_os = "Unix";
#else
static constexpr const char* s_os = "\\u00AF\\\\_(\\u30C4)_\\/\\u00AF";  //shrug I dunno
#endif
//copy paste end

using namespace fmt::literals;

template<typename rng, typename U>
//[[maybe_unused]]
constexpr bool erase_first_quick(rng& range, U&& val) {
	const auto it = std::find(range.begin(), range.end(), std::forward<U>(val));
	if (it == range.end())
		return false;
	std::swap(range.back(), *it);
	range.pop_back();
	return true;
}

namespace cacheless {
template<typename T>
T* ptr_or_null(ref_stable_map<snowflake, T>& in, snowflake key) {
	if (key.val)
		return &in[key];
	return nullptr;
}


internal_shard::internal_shard(
	int shard_number, client* t_parent, boost::asio::io_context& ioc, std::string_view gateway, intents intent
) :
	shard(shard_number, t_parent, ioc, std::string(t_parent->auth_token())),
	m_heartbeat_context(*this),
	m_ioc(ioc),
	m_resolver(ioc),
	m_socket(boost::asio::any_io_executor(strand()), m_ssl_ctx),
	m_intents(intent) {
	init_shard(shard_number, *this, ioc, gateway);
}

nlohmann::json internal_shard::presence() const {
	nlohmann::json retVal;
	retVal["since"];
	retVal["game"]["name"] = m_game_name;
	retVal["game"]["type"] = 0;
	retVal["status"] = enum_to_string(m_status);
	retVal["afk"] = m_status == Status::idle;

	return retVal;
}

cerwy::eager_task<voice_connection> internal_shard::connect_voice(const voice_channel& channel) {
	const auto channel_id = channel.id;
	const auto guild_id = channel.guild_id;
	auto [endpoint, session_id_task] = m_opcode4(channel.guild_id, channel_id);
	auto ep_json = co_await endpoint;
	auto session_id = co_await session_id_task;
	std::cout << ep_json.dump(1);
	std::string gateway = "wss://"s + ep_json["endpoint"].get<std::string>() + "/?v=4"s;
	std::cout << gateway << std::endl;

	m_things_waiting_for_voice_endpoint2.erase(channel.guild_id);
	m_things_waiting_for_voice_endpoint.erase(channel.guild_id);
	co_return co_await voice_connect_impl(*this, channel, std::move(gateway), ep_json["token"].get<std::string>(), std::move(session_id));
}

cerwy::eager_task<voice_connection> internal_shard::connect_voice(snowflake guild_id, snowflake channel_id) {
	auto [endpoint, session_id_task] = m_opcode4(guild_id, channel_id);
	auto ep_json = co_await endpoint;
	auto session_id = co_await session_id_task;
	std::cout << ep_json.dump(1);
	std::string gateway = "wss://"s + ep_json["endpoint"].get<std::string>() + "/?v=4"s;
	std::cout << gateway << std::endl;

	m_things_waiting_for_voice_endpoint2.erase(guild_id);
	m_things_waiting_for_voice_endpoint.erase(guild_id);
	co_return co_await voice_connect_impl(*this, guild_id, channel_id, std::move(gateway), ep_json["token"].get<std::string>(), std::move(session_id));
}

cerwy::eager_task<void> internal_shard::send_identity() const {
	auto identity = fmt::format(R"(
{{
"op":2,
"d":{{
	"token":"{}",
	"properties":{{
		"$os":"{}",
		"$browser":"cerwy",
		"$device":"cerwy"
	}},
	"compress":false,
	"large_threashold":{},
	"shard":[{},{}],
	"presence":{},
	"intents":{}
}}
}}
)", m_parent_client->token(), s_os, large_threshold, m_shard_number, m_parent_client->num_shards(), presence().dump(), m_intents.int_value());

	// auto timer = std::make_unique<boost::asio::steady_timer>(m_ioc);
	// timer->expires_at(m_parent_client->get_time_point_for_identifying());
	// timer->async_wait([pin = std::move(timer), str = std::move(identity),this](auto ec)mutable {
	// 	m_web_socket->send_thing(std::move(str));
	// });

	co_await m_web_socket->send_thing(std::move(identity));
	m_parent_client->notify_identified();
}

cerwy::eager_task<boost::beast::error_code> internal_shard::connect_http_connection() {
	auto ec = co_await m_http_connection.async_connect();
	int tries = 1;
	//TODO: do something else
	while (ec && tries < 10) {
		ec = co_await m_http_connection.async_connect();
		++tries;
	}
	co_return ec;
}

void internal_shard::doStuff(nlohmann::json stuffs, int op) {
	//fmt::print("{}\n", stuffs.dump(1));
	switch (op) {
	case 0:
		m_opcode0(std::move(stuffs["d"]), to_event_name(stuffs["t"]), stuffs["s"]);
		break;
	case 1:
		m_opcode1_send_heartbeat();
		break;
	case 2: break;
	case 3: break;
	case 4: break;
	case 5: break;
	case 6: break;
	case 7:
		m_opcode7_reconnect();
		break;
	case 8: break;
	case 9:
		m_opcode9_on_invalid_session(std::move(stuffs["d"]));
		break;
	case 10:
		m_opcode10_on_hello(stuffs["d"]);
		break;
	case 11:
		m_opcode11(stuffs["d"]);
		break;
	default: break;
	}
}

// ReSharper disable CppMemberFunctionMayBeConst
void internal_shard::on_reconnect() {
	// ReSharper restore CppMemberFunctionMayBeConst
	send_resume();
}

void internal_shard::rate_limit(std::chrono::system_clock::time_point tp) {
	m_http_connection.sleep_till(tp);
}

void internal_shard::close_connection(int code) {
	m_is_disconnected = true;
	m_web_socket->close(boost::beast::websocket::close_reason(code));
}


void internal_shard::request_guild_members(snowflake guild_id) const {
	m_opcode8_guild_member_chunk(guild_id);
}

void internal_shard::m_opcode0(nlohmann::json data, event_name event, uint64_t s) {
	m_seq_num.store(std::max(s, m_seq_num.load(std::memory_order_relaxed)), std::memory_order_relaxed);
	std::cout << m_seq_num << std::endl;
	//std::cout << data << std::endl;
	//{
	//auto lock = co_await m_events_mut.async_lock();//?
	//}
	try {
		switch (event) {
		case event_name::HELLO: procces_event<event_name::HELLO>(data);
			break;
		case event_name::READY: procces_event<event_name::READY>(data);
			break;;
		case event_name::RESUMED: procces_event<event_name::RESUMED>(data);
			break;;
		case event_name::INVALID_SESSION: procces_event<event_name::INVALID_SESSION>(data);
			break;;
		case event_name::CHANNEL_CREATE: procces_event<event_name::CHANNEL_CREATE>(data);
			break;;
		case event_name::CHANNEL_UPDATE: procces_event<event_name::CHANNEL_UPDATE>(data);
			break;;
		case event_name::CHANNEL_DELETE: procces_event<event_name::CHANNEL_DELETE>(data);
			break;;
		case event_name::CHANNEL_PINS_UPDATE: procces_event<event_name::CHANNEL_PINS_UPDATE>(data);
			break;;
		case event_name::GUILD_CREATE: procces_event<event_name::GUILD_CREATE>(data);
			break;;
		case event_name::GUILD_UPDATE: procces_event<event_name::GUILD_UPDATE>(data);
			break;;
		case event_name::GUILD_DELETE: procces_event<event_name::GUILD_DELETE>(data);
			break;;
		case event_name::GUILD_BAN_ADD: procces_event<event_name::GUILD_BAN_ADD>(data);
			break;;
		case event_name::GUILD_BAN_REMOVE: procces_event<event_name::GUILD_BAN_REMOVE>(data);
			break;;
		case event_name::GUILD_EMOJI_UPDATE: procces_event<event_name::GUILD_EMOJI_UPDATE>(data);
			break;;
		case event_name::GUILD_INTEGRATIONS_UPDATE: procces_event<event_name::GUILD_INTEGRATIONS_UPDATE>(data);
			break;;
		case event_name::GUILD_MEMBER_ADD: procces_event<event_name::GUILD_MEMBER_ADD>(data);
			break;;
		case event_name::GUILD_MEMBER_REMOVE: procces_event<event_name::GUILD_MEMBER_REMOVE>(data);
			break;;
		case event_name::GUILD_MEMBER_UPDATE: procces_event<event_name::GUILD_MEMBER_UPDATE>(data);
			break;;
		case event_name::GUILD_MEMBERS_CHUNK: procces_event<event_name::GUILD_MEMBERS_CHUNK>(data);
			break;;
		case event_name::GUILD_ROLE_CREATE: procces_event<event_name::GUILD_ROLE_CREATE>(data);
			break;;
		case event_name::GUILD_ROLE_UPDATE: procces_event<event_name::GUILD_ROLE_UPDATE>(data);
			break;;
		case event_name::GUILD_ROLE_DELETE: procces_event<event_name::GUILD_ROLE_DELETE>(data);
			break;;
		case event_name::MESSAGE_CREATE: procces_event<event_name::MESSAGE_CREATE>(data);
			break;;
		case event_name::MESSAGE_UPDATE: procces_event<event_name::MESSAGE_UPDATE>(data);
			break;;
		case event_name::MESSAGE_DELETE: procces_event<event_name::MESSAGE_DELETE>(data);
			break;;
		case event_name::MESSAGE_DELETE_BULK: procces_event<event_name::MESSAGE_DELETE_BULK>(data);
			break;;
		case event_name::MESSAGE_REACTION_ADD: procces_event<event_name::MESSAGE_REACTION_ADD>(data);
			break;;
		case event_name::MESSAGE_REACTION_REMOVE: procces_event<event_name::MESSAGE_REACTION_REMOVE>(data);
			break;;
		case event_name::MESSAGE_REACTION_REMOVE_ALL: procces_event<event_name::MESSAGE_REACTION_REMOVE_ALL>(data);
			break;;
		case event_name::PRESENCE_UPDATE: procces_event<event_name::PRESENCE_UPDATE>(data);
			break;;
		case event_name::TYPING_START: procces_event<event_name::TYPING_START>(data);
			break;;
		case event_name::USER_UPDATE: procces_event<event_name::USER_UPDATE>(data);
			break;;
		case event_name::VOICE_STATE_UPDATE: procces_event<event_name::VOICE_STATE_UPDATE>(data);
			break;;
		case event_name::VOICE_SERVER_UPDATE: procces_event<event_name::VOICE_SERVER_UPDATE>(data);
			break;;
		case event_name::WEBHOOKS_UPDATE: procces_event<event_name::WEBHOOKS_UPDATE>(data);
			break;;

		}
	} catch (...) {
		//swallow
	}
}

void internal_shard::m_opcode1_send_heartbeat() const {
	if (is_disconnected())
		return;
	std::string t =
			R"({
	"op": 1,
	"d" : null
})";
	m_web_socket->send_thing(std::move(t));
}

void internal_shard::m_opcode2_send_identity() const {
	m_send_identity();
}

void internal_shard::m_opcode3_send_presence() const {
	nlohmann::json val;
	val["op"] = 3;
	val["data"] = presence();
	m_web_socket->send_thing(val.dump());
}

std::pair<cerwy::eager_task<nlohmann::json>, cerwy::eager_task<std::string>> internal_shard::m_opcode4(snowflake guild_id, snowflake channel_id) {
	cerwy::promise<nlohmann::json> name_promise;
	cerwy::promise<std::string> session_id_promise;

	auto name = name_promise.get_task();
	auto session_id_task = session_id_promise.get_task();

	m_things_waiting_for_voice_endpoint.insert(std::make_pair(guild_id, std::move(name_promise)));
	m_things_waiting_for_voice_endpoint2.insert(std::make_pair(guild_id, std::move(session_id_promise)));

	m_web_socket->send_thing(
		R"(
{{
    "op": 4,
    "d": {{
        "guild_id": "{}",
        "channel_id": "{}",
        "self_mute": false,
        "self_deaf": false
    }}
}}
)"_format(guild_id, channel_id));


	return {std::move(name), std::move(session_id_task)};
}

void internal_shard::m_opcode6_send_resume() const {
	send_resume();
}

void internal_shard::m_opcode7_reconnect() {
	reconnect();
}

void internal_shard::m_opcode8_guild_member_chunk(snowflake id) const {
	nlohmann::json s;
	s["op"] = 8;
	s["d"]["guild_id"] = std::to_string(id.val);
	s["d"]["query"] = "";
	s["d"]["limit"] = 0;
	std::cout << s << std::endl;
	m_web_socket->send_thing(s.dump());
}

cerwy::eager_task<void> internal_shard::m_opcode9_on_invalid_session(nlohmann::json d) {
	boost::asio::steady_timer timer(strand(), std::chrono::steady_clock::now() + 5s);
	auto ec = co_await timer.async_wait(use_task_return_ec);
	//co_await resume_on_strand(strand());

	if (d.get<bool>())
		m_opcode6_send_resume();
	else
		m_opcode2_send_identity();

	/*
	std::this_thread::sleep_for(5s);

	if (d.get<bool>())
		m_opcode6_send_resume();
	else
		m_opcode2_send_identity();

	*/
}


void internal_shard::m_opcode10_on_hello(nlohmann::json& stuff) {
	m_heartbeat_context.hb_interval = stuff["heartbeat_interval"].get<int>();
	m_heartbeat_context.start();
	//m_send_identity();
	m_parent_client->queue_to_identify(*this);
}

void internal_shard::m_opcode11(nlohmann::json& data) {
	//m_op11 = true;
	m_heartbeat_context.recived_ack = true;
}

void internal_shard::reconnect() {
	if (!is_disconnected())
		close_connection(1001);
	//m_parent->reconnect(this, m_shard_number);
	//reconnect will happen in init_shard
}

void internal_shard::send_resume() const {
	std::string temp(R"({
		"op":6,
		"d":{
			"token":)"s + std::string(m_parent_client->token()) + R"(
			"session_id:)"s + m_session_id + R"(
			"seq:")" + std::to_string(m_seq_num) + R"(
		}
	}
	)"s);
	m_web_socket->send_thing(std::move(temp));
}

void internal_shard::apply_presences(const nlohmann::json& presences, Guild& guild) {
	for (const auto& presence : presences) {
		const auto id = presence["user"]["id"].get<snowflake>();
		const auto& temp = presence["game"];
		if (!temp.is_null()) {
			guild.activities.insert(std::make_pair(id, std::optional<activity>(temp.get<activity>())));
		}
		//guild.status[id] = string_to_status(presence["status"].get_ref<const nlohmann::json::string_t&>());
	}
}

void internal_shard::replay_events_for(snowflake guild_id) {
	auto it = m_backed_up_events.find(guild_id);
	if (it != m_backed_up_events.end()) {
		auto backed_up_events = std::move(it->second);
		m_backed_up_events.erase(it);
		for (auto& event : backed_up_events) {
			m_opcode0(event.first, event.second, 0);
		}
	} else {
		//???????????
	}
}

void internal_shard::m_send_identity() const {
	auto identity = fmt::format(R"(
{{
"op":2,
"d":{{
	"token":"{}",
	"properties":{{
		"$os":"{}",
		"$browser":"cerwy",
		"$device":"cerwy"
	}},
	"compress":false,
	"large_threashold":{},
	"shard":[{},{}],
	"presence":{},
	"intents":{}
}}
}}
)", m_parent_client->token(), std::string_view(s_os), large_threshold, m_shard_number, m_parent_client->num_shards(), presence().dump(), m_intents.int_value());

	auto timer = std::make_unique<boost::asio::steady_timer>(m_ioc);
	timer->expires_at(m_parent_client->get_time_point_for_identifying());
	timer->async_wait([pin = std::move(timer), str = std::move(identity),this](auto ec)mutable {
		m_web_socket->send_thing(std::move(str));
	});
}

template<>
void internal_shard::procces_event<event_name::READY>(nlohmann::json& event) {

	m_trace2 = event["_trace"];
	auto me = event["user"].get<user>();
	m_session_id = event["session_id"].get<std::string>();	
	//ignore unavailable guilds, only need size
	m_guilds.reserve(event["guilds"].size());

	m_id = me.id;
	if (m_parent_client->on_ready.has_value()) {
		m_parent_client->on_ready.value()(*this);
	}
}

template<>
void internal_shard::procces_event<event_name::GUILD_CREATE>(nlohmann::json& data) {
	// std::cout << data["id"] << std::endl;
	// Guild& guild = m_guilds.insert(std::make_pair(data["id"].get<snowflake>(), data.get<Guild>())).first->second;
	// if (m_intents.has_intents(intent::GUILD_MEMBERS) && guild.m_member_count >= large_threshold) {
	// 	guild.m_is_ready = false;
	// 	request_guild_members(guild);
	// } else if (m_intents.has_intents(intent::GUILD_MEMBERS)) {
	// 	for (const auto& member_json : data["members"]) {
	// 		auto member = member_json.get<guild_member>();
	// 		member.m_guild = &guild;
	// 		const auto id = member.id();
	// 		guild.m_members.insert(std::make_pair(id, std::move(member)));
	// 	}
	// }
	//
	// const auto channels = data["channels"].get_ref<const nlohmann::json::array_t&>();
	// guild.m_text_channel_ids.reserve(channels.size());
	// guild.m_voice_channel_ids.reserve(std::max(3ull, channels.size() / 10));//random numbers
	//
	// for (const auto& channel_json : channels) {
	// 	if (const auto type = channel_json["type"].get<int>(); type == 0 || type == 5 || type == 6) {//text
	// 		auto& channel = m_text_channel_map.insert(std::make_pair(channel_json["id"].get<snowflake>(), channel_json.get<text_channel>())).first->second;
	// 		channel.m_guild_id = guild.id();
	// 		guild.m_text_channel_ids.push_back(channel.id());
	// 		channel.m_guild = &guild;
	// 		if (type == 5) {
	// 			channel.m_channel_type = text_channel_type::news;
	// 		}
	//
	// 		if (type == 6) {
	// 			channel.m_channel_type = text_channel_type::store;
	// 		}
	// 	} else if (type == 2) {//voice
	// 		auto& channel = m_voice_channel_map.insert(std::make_pair(channel_json["id"].get<snowflake>(), channel_json.get<voice_channel>())).first->second;
	// 		channel.m_guild_id = guild.id();
	// 		guild.m_voice_channel_ids.push_back(channel.id());
	// 		channel.m_guild = &guild;
	// 	} else if (type == 4) {//guild catagory
	// 		auto& channel = m_channel_catagory_map.insert(std::make_pair(channel_json["id"].get<snowflake>(), channel_json.get<channel_catagory>())).first->second;
	// 		channel.m_guild_id = guild.id();
	// 		guild.m_channel_catagory_ids.push_back(channel.id());
	// 		channel.m_guild = &guild;
	// 	} else {
	// 		//unimplemented channel
	// 	}
	//
	// 	//type == 1,3 is missing since it's DM channel, dm channels don't exist in guild channels
	// }
	//
	// for (auto& channel : guild.m_text_channel_ids |
	// 	 ranges::views::transform(hof::map_with(m_text_channel_map)) |
	// 	 ranges::views::filter(&text_channel::has_parent)) {
	// 	channel.m_parent = &m_channel_catagory_map[channel.m_parent_id];
	// }
	//
	// for (const auto& channel_id : guild.m_voice_channel_ids) {
	// 	auto& channel = m_voice_channel_map[channel_id];
	// 	if (channel.m_parent_id.val)
	// 		channel.m_parent = &m_channel_catagory_map[channel.m_parent_id];
	// }
	//
	// if (m_intents.has_intents(intent::GUILD_PRESENCES)) {
	// 	if (guild.m_is_ready) {
	// 		apply_presences(data["presences"], guild);
	// 	} else {
	// 		guild.m_presences = std::move(data["presences"]);//save it for later
	// 	}
	// }
	// guild.m_shard = this;
	// if (guild.m_is_ready) {
	// 	m_parent->on_guild_ready(guild, *this);
	// }

	auto guild = data.get<Guild>();
	events::guild_create event;
	event.guild = std::move(guild);
	//m_parent->on_guild_ready(std::move(guild), *this);
	if(m_parent_client->on_guild_ready.has_value()) {
		m_parent_client->on_guild_ready.value()(std::move(event), *this);
	}

}

template<>
void internal_shard::procces_event<event_name::GUILD_MEMBERS_CHUNK>(nlohmann::json& e) {

	
	//m_parent->on_guild_member_chunk( guild_id, std::move(members), chunk_count, chunk_index, *this);
	events::guild_members_chunk event;
	event.members = e["members"].get<std::vector<guild_member>>();
	event.chunk_count = e["chunk_count"].get<int>();
	e["chunk_index"].get_to(event.chunk_index);
	e["guild_id"].get_to(event.guild_id);
	e["presences"].get_to(event.presences);
}


template<>
void internal_shard::procces_event<event_name::MESSAGE_CREATE>(nlohmann::json& e) {
	const bool is_guild_msg = e.contains("guild_id");
	if (is_guild_msg) {

		events::guild_message_create event;
		event.msg = create_msg<guild_text_message>(e);
		if (m_parent_client->on_guild_text_msg.has_value()) {
			m_parent_client->on_guild_text_msg.value()(std::move(event), *this);
		}
	} else {//dm msg

		events::dm_message_create event;
		event.msg = create_msg<dm_message>(e);
		if (m_parent_client->on_dm_msg.has_value()) {
			m_parent_client->on_dm_msg.value()(std::move(event), *this);
		}
	}
}

template<>
void internal_shard::procces_event<event_name::CHANNEL_CREATE>(nlohmann::json& channel_json) {
	const auto type = channel_json["type"].get<int>();
	if (type == 0 || type == 5 || type == 6) {//text


		events::text_channel_create event;
		auto& channel = event.channel = channel_json.get<text_channel>();// guild.text_channels.insert(std::make_pair(channel_json["id"].get<snowflake>(), )).first->second;

		if (type == 5) {
			channel.channel_type = text_channel_type::news;
		}

		if (type == 6) {
			channel.channel_type = text_channel_type::store;
		}
		if (m_parent_client->on_guild_text_channel_create.has_value()) {
			m_parent_client->on_guild_text_channel_create.value()(std::move(event), *this);
		}
	} else if (type == 1 || type == 3) {//DM
		if (m_parent_client->on_dm_channel_create.has_value()) {
			m_parent_client->on_dm_channel_create.value()({ channel_json.get<dm_channel>() }, *this);
		}
	} else if (type == 2) {//voice
		if (m_parent_client->on_guild_voice_channel_create.has_value()) {
			m_parent_client->on_guild_voice_channel_create.value()({ channel_json.get<voice_channel>() }, *this);
		}
	} else if (type == 4) {//guild catagory
		if (m_parent_client->on_guild_channel_catagory_create.has_value()) {
			m_parent_client->on_guild_channel_catagory_create.value()({ channel_json.get<channel_catagory>() }, *this);
		}
	} else {
		//unimplemented channel
	}


}

template<>
void internal_shard::procces_event<event_name::CHANNEL_DELETE>(nlohmann::json& e) {
	const auto type = e["type"].get<int>();
	if (type == 0 || type == 5 || type == 6) {//text

		events::text_channel_delete event;
		auto& channel = event.channel = e.get<text_channel>();// guild.text_channels.insert(std::make_pair(channel_json["id"].get<snowflake>(), )).first->second;

		if (type == 5) {
			channel.channel_type = text_channel_type::news;
		}

		if (type == 6) {
			channel.channel_type = text_channel_type::store;
		}
		if (m_parent_client->on_guild_text_channel_delete.has_value()) {
			m_parent_client->on_guild_text_channel_delete.value()(std::move(event), *this);
		}
	} else if (type == 1 || type == 3) {//DM
		events::dm_channel_delete event;
		event.channel = e.get<dm_channel>();
		if (m_parent_client->on_dm_channel_delete.has_value()) {
			m_parent_client->on_dm_channel_delete.value()(std::move(event), *this);
		}
	} else if (type == 2) {//voice

		events::voice_channel_delete event;
		event.channel = e.get<voice_channel>();
		if (m_parent_client->on_guild_voice_channel_delete.has_value()) {
			m_parent_client->on_guild_voice_channel_delete.value()(std::move(event), *this);
		}
	} else if (type == 4) {//guild catagory
		events::channel_catagory_delete event;
		event.channel = e.get<channel_catagory>();
		if (m_parent_client->on_guild_channel_catagory_delete.has_value()) {
			m_parent_client->on_guild_channel_catagory_delete.value()(std::move(event), *this);
		}
	} else {
		//unimplemented channel
	}

}

template<>
void internal_shard::procces_event<event_name::GUILD_MEMBER_ADD>(nlohmann::json& e) {

	events::guild_member_add event;
	event.guild_id = e["guild_id"].get<snowflake>();
	event.member = e.get<guild_member>();
	if (m_parent_client->on_guild_member_add.has_value()) {
		m_parent_client->on_guild_member_add.value()(std::move(event), *this);
	}
}

template<>
void internal_shard::procces_event<event_name::GUILD_MEMBER_REMOVE>(nlohmann::json& e) {
	events::guild_member_remove event;
	event.guild_id = e["guild_id"].get<snowflake>();
	event.member = e["user"].get<user>();
	if (m_parent_client->on_guild_member_remove.has_value()) {
		m_parent_client->on_guild_member_remove.value()(std::move(event), *this);
	}
}

template<>
void internal_shard::procces_event<event_name::RESUMED>(nlohmann::json& e) {
	m_trace = e["_trace"].get<std::vector<std::string>>();
};

template<>
void internal_shard::procces_event<event_name::HELLO>(nlohmann::json& json) {
	m_trace = json.at("_trace").get<std::vector<std::string>>();
	m_heartbeat_context.hb_interval = json["heartbeat_interval"].get<int>();
};

template<>
void internal_shard::procces_event<event_name::INVALID_SESSION>(nlohmann::json&) { };

template<>
void internal_shard::procces_event<event_name::CHANNEL_UPDATE>(nlohmann::json& e) {
	const auto type = e["type"].get<int>();
	if (type == 0 || type == 5 || type == 6) {//text

		events::text_channel_update event;
		auto& channel = event.channel = e.get<text_channel>();// guild.text_channels.insert(std::make_pair(channel_json["id"].get<snowflake>(), )).first->second;


		if (type == 5) {
			channel.channel_type = text_channel_type::news;
		}

		if (type == 6) {
			channel.channel_type = text_channel_type::store;
		}
		if (m_parent_client->on_guild_text_channel_update.has_value()) {
			m_parent_client->on_guild_text_channel_update.value()(std::move(event), *this);
		}
	} else if (type == 1 || type == 3) {//DM
		if (m_parent_client->on_dm_channel_update.has_value()) {
			events::dm_channel_update event;
			event.channel = e.get<dm_channel>();
			m_parent_client->on_dm_channel_update.value()(std::move(event), *this);
		}

	} else if (type == 2) {//voice

		events::voice_channel_update event;
		event.channel = e.get<voice_channel>();
		if (m_parent_client->on_guild_voice_channel_update.has_value()) {
			m_parent_client->on_guild_voice_channel_update.value()(std::move(event), *this);
		}
	} else if (type == 4) {//guild catagory
		events::channel_catagory_update event;
		event.channel = e.get<channel_catagory>();
		if (m_parent_client->on_guild_channel_catagory_update.has_value()) {
			m_parent_client->on_guild_channel_catagory_update.value()(std::move(event), *this);
		}
	} else {
		//unimplemented channel
	}
}

template<>
void internal_shard::procces_event<event_name::CHANNEL_PINS_UPDATE>(nlohmann::json& e) {};

template<>
void internal_shard::procces_event<event_name::GUILD_UPDATE>(nlohmann::json& e) {
	const auto guild_id = e["id"].get<snowflake>();

	auto guild = e.get<partial_guild>();
	//m_parent->on_guild_update(g, *this);
};

template<>
void internal_shard::procces_event<event_name::GUILD_DELETE>(nlohmann::json& e) {
	try {
		events::guild_delete event;
		event.guild = e.get<unavailable_guild>();
		if (m_parent_client->on_guild_remove.has_value()) {
			m_parent_client->on_guild_remove.value()(event, *this);
		}
	} catch (...) {
		//swallow		 
	}
}

template<>
void internal_shard::procces_event<event_name::GUILD_BAN_ADD>(nlohmann::json& e) {
	events::guild_ban_add event;
	event.user = e.get<user>();
	event.guild_id = e["guild_id"].get<snowflake>();
	if (m_parent_client->on_ban_add.has_value()) {
		m_parent_client->on_ban_add.value()(std::move(event), *this);
	}
};

template<>
void internal_shard::procces_event<event_name::GUILD_BAN_REMOVE>(nlohmann::json& e) {
	events::guild_ban_remove event;
	event.user = e.get<user>();
	event.guild_id = e["guild_id"].get<snowflake>();
	if (m_parent_client->on_ban_remove.has_value()) {
		m_parent_client->on_ban_remove.value()(std::move(event), *this);
	}
}

template<>
void internal_shard::procces_event<event_name::GUILD_EMOJI_UPDATE>(nlohmann::json& e) {
	events::guild_emoji_update event;
	event.guild_id = e["guild_id"].get<snowflake>();
	event.emojis = e["emojis"].get<std::vector<emoji>>();
	if (m_parent_client->on_emoji_update.has_value()) {
		m_parent_client->on_emoji_update.value()(std::move(event), *this);
	}
};

template<>
void internal_shard::procces_event<event_name::GUILD_INTEGRATIONS_UPDATE>(nlohmann::json& e) {
	//what is this ;-;
	//only has 1 field: id
	//wat
	events::guild_integration_update event;
	event.guild_id = e["id"].get<snowflake>();

};

template<>
void internal_shard::procces_event<event_name::GUILD_MEMBER_UPDATE>(nlohmann::json& e) {

	events::guild_member_update event;
	event.guild_id = e["guild_id"].get<snowflake>();

	event.user = e["user"].get<cacheless::user>();
	event.roles = e["roles"].get<std::vector<guild_role>>();
	if (e.contains("nick")) {
		event.nick = e["nick"].get<std::string>();
	}
	if (m_parent_client->on_guild_member_update.has_value()) {
		m_parent_client->on_guild_member_update.value()(std::move(event), *this);
	}
};

template<>
void internal_shard::procces_event<event_name::GUILD_ROLE_CREATE>(nlohmann::json& e) {

	events::guild_role_create event;
	event.guild_id = e["guild_id"].get<snowflake>();
	event.role = e["role"].get<guild_role>();
	if (m_parent_client->on_role_create.has_value()) {
		m_parent_client->on_role_create.value()(std::move(event), *this);
	}
	//m_parent->on_role_create(guild, guild.m_roles.insert(std::make_pair(e["role"]["id"].get<snowflake>(), )).first->second, *this);
	//m_parent->on_role_create(guild_id, std::move(new_role),*this);
};

template<>
void internal_shard::procces_event<event_name::GUILD_ROLE_UPDATE>(nlohmann::json& e) {

	events::guild_role_update event;
	event.guild_id = e["guild_id"].get<snowflake>();
	event.role = e["role"].get<guild_role>();
	if (m_parent_client->on_role_update.has_value())
		m_parent_client->on_role_update.value()(std::move(event), *this);
};

template<>
void internal_shard::procces_event<event_name::GUILD_ROLE_DELETE>(nlohmann::json& e) {

	events::guild_role_delete event;

	event.guild_id = e["guild_id"].get<snowflake>();
	event.role_id = e["role_id"].get<snowflake>();
	if (m_parent_client->on_role_delete.has_value()) {
		m_parent_client->on_role_delete.value()(event, *this);
	}
};

template<>
void internal_shard::procces_event<event_name::MESSAGE_UPDATE>(nlohmann::json& e) {

	const bool is_guild_msg = e.contains("guild_id");
	if (is_guild_msg) {

		auto msg = createMsgUpdate<guild_msg_update>(e);

		//m_parent->on_guild_msg_update(e["guild_id"].get<snowflake>(),std::move(msg), *this);
	} else {//dm msg

		auto msg = createMsgUpdate<dm_msg_update>(e);
		//m_parent->on_dm_msg_update(std::move(msg), *this);
	}
}

template<>
void internal_shard::procces_event<event_name::MESSAGE_DELETE>(nlohmann::json& e) {
	const auto id = e["id"].get<snowflake>();
	const auto channel_id = e["channel_id"].get<snowflake>();
	auto guild_id = e.value("guild_id", snowflake());
	if (guild_id != snowflake()) {
		events::guild_message_delete event;
		event.id = id;
		event.guild_id = guild_id;
		event.channel_id = channel_id;
		if (m_parent_client->on_guild_msg_delete.has_value()) {
			m_parent_client->on_guild_msg_delete.value()(event, *this);
		}
	} else {

		events::dm_message_delete event;
		event.id = id;
		event.channel_id = channel_id;
		if (m_parent_client->on_dm_msg_delete.has_value()) {
			m_parent_client->on_dm_msg_delete.value()(event, *this);
		}
	}
};

template<>
void internal_shard::procces_event<event_name::MESSAGE_DELETE_BULK>(nlohmann::json& e) {
	auto msg_ids = e["ids"].get<std::vector<snowflake>>();
	const auto channel_id = e["channel_id"].get<snowflake>();
	const auto guild_id = e.value("guild_id", snowflake());
	if (guild_id != snowflake()) {
		events::guild_message_delete_bulk event;
		event.ids = std::move(msg_ids);
		event.channel_id = channel_id;
		event.guild_id = guild_id;
		if (m_parent_client->on_message_bulk_delete.has_value()) {
			m_parent_client->on_message_bulk_delete.value()(std::move(event), *this);
		}
	} else {
		events::dm_message_delete_bulk event;
		event.ids = std::move(msg_ids);
		event.channel_id = channel_id;
		if (m_parent_client->on_dm_message_bulk_delete.has_value()) {
			m_parent_client->on_dm_message_bulk_delete.value()(std::move(event), *this);
		}
	}
};

template<>
void internal_shard::procces_event<event_name::MESSAGE_REACTION_ADD>(nlohmann::json& e) {
	const auto channel_id = e["channel_id"].get<snowflake>();
	auto emoji = e["emoji"].get<partial_emoji>();
	const snowflake guild_id = e.value("guild_id", snowflake());
	const auto message_id = e["message_id"].get<snowflake>();

	if (guild_id != snowflake()) {
		events::guild_message_reaction_add event;
		event.channel_id = channel_id;
		event.emoji = std::move(emoji);
		event.guild_id = guild_id;
		event.message_id = message_id;
		event.member = e["member"].get<guild_member>();
		if (m_parent_client->on_guild_reaction_add.has_value()) {
			m_parent_client->on_guild_reaction_add.value()(std::move(event), *this);
		}
	} else {
		events::dm_message_reaction_add event;
		event.channel_id = channel_id;
		event.emoji = std::move(emoji);
		event.message_id = message_id;
		event.user_id = e["user_id"].get<snowflake>();
		if (m_parent_client->on_dm_reaction_add.has_value()) {
			m_parent_client->on_dm_reaction_add.value()(std::move(event), *this);;
		}
	}
};

template<>
void internal_shard::procces_event<event_name::MESSAGE_REACTION_REMOVE>(nlohmann::json& e) {
	const auto channel_id = e["channel_id"].get<snowflake>();
	auto emojiy = e["emoji"].get<partial_emoji>();
	const bool is_guild = e.contains("guild_id");
	const auto message_id = e["message_id"].get<snowflake>();

	if (is_guild) {
		events::guild_message_reaction_remove event;
		event.channel_id = channel_id;
		event.emoji = std::move(emojiy);
		event.user_id = e["user_id"].get<snowflake>();
		event.message_id = message_id;
		event.guild_id = e["guild_id"].get<snowflake>();
		if (m_parent_client->on_guild_reaction_remove.has_value()) {
			m_parent_client->on_guild_reaction_remove.value()(std::move(event), *this);
		}
	} else {
		events::dm_message_reaction_remove event;
		event.channel_id = channel_id;
		event.emoji = std::move(emojiy);
		event.message_id = message_id;
		event.user_id = e["user_id"].get<snowflake>();
		if (m_parent_client->on_dm_reaction_remove.has_value()) {
			m_parent_client->on_dm_reaction_remove.value()(std::move(event), *this);
		}

	}
};

template<>
void internal_shard::procces_event<event_name::MESSAGE_REACTION_REMOVE_ALL>(nlohmann::json& e) {
	const auto channel_id = e["channel_id"].get<snowflake>();
	const auto message_id = e["message_id"].get<snowflake>();
	const auto guild_id = e.value("guild_id", snowflake());

	if (guild_id.val != 0) {
		events::guild_message_reaction_remove_all event;
		event.channel_id = channel_id;
		event.message_id = message_id;
		event.guild_id = guild_id;
		if (m_parent_client->on_guild_reaction_remove_all.has_value()) {
			m_parent_client->on_guild_reaction_remove_all.value()(event, *this);
		}
	} else {
		events::dm_message_reaction_remove_all event;
		event.channel_id = channel_id;
		event.message_id = message_id;
		if (m_parent_client->on_dm_reaction_remove_all.has_value()) {
			m_parent_client->on_dm_reaction_remove_all.value()(event, *this);
		}
	}

};

template<>
void internal_shard::procces_event<event_name::PRESENCE_UPDATE>(nlohmann::json& e) {
	auto update_event = e.get<events::presence_update_event>();
	if (m_parent_client->on_presence_update.has_value())
		m_parent_client->on_presence_update.value()(std::move(update_event), *this);
};

template<>
void internal_shard::procces_event<event_name::TYPING_START>(nlohmann::json& e) {
	const auto channel_id = e["channel_id"].get<snowflake>();
	const auto guild_id = e.value("guild_id", snowflake());

	if (guild_id != snowflake()) {
		events::guild_typing_start event;
		event.channel_id = channel_id;
		event.guild_id = guild_id;
		event.member = e["member"].get<guild_member>();
		if (m_parent_client->on_guild_typing_start.has_value()) {
			m_parent_client->on_guild_typing_start.value()(std::move(event), *this);
		}
	} else {
		events::dm_typing_start event;
		event.user_id = e["user_id"].get<snowflake>();
		event.channel_id = channel_id;
		if (m_parent_client->on_dm_typing_start.has_value()) {
			m_parent_client->on_dm_typing_start.value()(event, *this);
		}
	}
};

template<>
void internal_shard::procces_event<event_name::USER_UPDATE>(nlohmann::json& e) {
	//m_self_user = e.get<user>();
};

template<>
void internal_shard::procces_event<event_name::VOICE_STATE_UPDATE>(nlohmann::json& e) {

	const auto guild_id = e.value("guild_id", snowflake());
	//Guild& guild = guild_id.val ? m_guilds[guild_id] : *m_voice_channel_map[e["channel_id"].get<snowflake>()].m_guild;

	const auto user_id = e.value("user_id", id());
	auto voice_state = e.get<cacheless::voice_state>();


	if (user_id == this->id()) {
		auto it = m_things_waiting_for_voice_endpoint2.find(guild_id);
		if (it != m_things_waiting_for_voice_endpoint2.end()) {
			it->second.set_value(e["session_id"].get<std::string>());
		}
	}

};

template<>
void internal_shard::procces_event<event_name::VOICE_SERVER_UPDATE>(nlohmann::json& json) {
	const auto guild_id = json["guild_id"].get<snowflake>();
	m_things_waiting_for_voice_endpoint[guild_id].set_value(std::move(json));
};

template<>
void internal_shard::procces_event<event_name::WEBHOOKS_UPDATE>(nlohmann::json&) { }

// ReSharper disable once CppMemberFunctionMayBeConst
void internal_shard::update_presence(const Status s, std::string g) {// NOLINT
	m_status = s;
	m_game_name = std::move(g);
	m_opcode3_send_presence();
}

}
