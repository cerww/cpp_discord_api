#include "internal_shard.h"
#include "client.h"
#include "voice_channel.h"
#include "dm_channel.h"
#include "init_shard.h"
#include "voice_connect_impl.h"
#include "executor_binder.h"
//#include <boost/beast/>

//copy paste start
#if defined(_WIN32) || defined(_WIN64)
static constexpr const char * s_os = "Windows";
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

template<typename T>
T* ptr_or_null(ref_stable_map<snowflake,T>& in,snowflake key) {
	if (key.val)
		return &in[key];
	return nullptr;
}

internal_shard::internal_shard(int shard_number, client* t_parent, boost::asio::io_context& ioc, std::string_view gateway, intents intent) :
	shard(shard_number,t_parent,ioc),
	m_ioc(ioc),
	m_resolver(ioc),
	m_socket(ioc,m_ssl_ctx),
	m_intents(intent)
{
	init_shard(shard_number, *this, ioc, gateway);	
}

nlohmann::json internal_shard::presence()const {
	nlohmann::json retVal;
	retVal["since"];
	retVal["game"]["name"] = m_game_name;
	retVal["game"]["type"] = 0;
	retVal["status"] = enum_to_string(m_status);
	retVal["afk"] = m_status == Status::idle;

	return retVal;
}

cerwy::task<voice_connection> internal_shard::connect_voice(const voice_channel& channel) {
	const auto channel_id = channel.id();
	const auto guild_id = channel.guild_id();
	auto [endpoint, session_id_task] = m_opcode4(channel);
	auto ep_json = co_await endpoint;
	auto session_id = co_await session_id_task;
	std::cout << ep_json.dump(1);
	std::string gateway = "wss://"s +  ep_json["endpoint"].get<std::string>() + "/?v=4"s;
	std::cout << gateway << std::endl;

	m_things_waiting_for_voice_endpoint2.erase(channel.guild_id());
	m_things_waiting_for_voice_endpoint.erase(channel.guild_id());
	int i = 0;
	co_return co_await voice_connect_impl(*this, channel, std::move(gateway), ep_json["token"].get<std::string>(), std::move(session_id));
}

cerwy::task<boost::beast::error_code> internal_shard::connect_http_connection() {
	auto ec = co_await m_http_connection.async_connect();
	int tries = 1;
	//TODO: do something else
	while(ec && tries <10) {
		ec = co_await m_http_connection.async_connect();
		++tries;
	}
	co_return ec;
}

void internal_shard::doStuff(nlohmann::json stuffs,int op) {
	//fmt::print("{}\n", stuffs.dump(1));
	switch (op) {
	case 0:
		m_opcode0(std::move(stuffs["d"]), to_event_name(stuffs["t"]), stuffs["s"]);
		break;
	case 1:
		m_opcode1_send_heartbeat();
		break;
	case 2:break;
	case 3:break;
	case 4:break;
	case 5:break;
	case 6:break;
	case 7:
		m_opcode7_reconnect();
		break;
	case 8:break;
	case 9:
		m_opcode9_on_invalid_session(stuffs["d"]);
		break;
	case 10:
		m_opcode10_on_hello(stuffs["d"]);
		break;
	case 11:
		m_opcode11(stuffs["d"]);
		break;
	default:break;
	}
}

void internal_shard::on_reconnect() {
	send_resume();
}

void internal_shard::rate_limit(std::chrono::system_clock::time_point tp) {
	m_http_connection.sleep_till(tp);
}

void internal_shard::close_connection(int code) {
	m_is_disconnected = true;
	m_client->close(boost::beast::websocket::close_reason(code));
}



void internal_shard::request_guild_members(Guild& g) const {
	m_opcode8_guild_member_chunk(g.id());
	g.m_members.clear();
	g.m_members.reserve(g.m_member_count);
}

void internal_shard::m_opcode0(nlohmann::json data, event_name event, size_t s) {
	m_seqNum = std::max(s, m_seqNum);
	std::cout << m_seqNum << std::endl;
	//std::cout << data << std::endl;
	try {
		switch (event) {
		case event_name::HELLO:							procces_event<event_name::HELLO>(data); break;
		case event_name::READY:							procces_event<event_name::READY>(data); break;;
		case event_name::RESUMED:						procces_event<event_name::RESUMED>(data); break;;
		case event_name::INVALID_SESSION:				procces_event<event_name::INVALID_SESSION>(data); break;;
		case event_name::CHANNEL_CREATE:				procces_event<event_name::CHANNEL_CREATE>(data); break;;
		case event_name::CHANNEL_UPDATE:				procces_event<event_name::CHANNEL_UPDATE>(data); break;;
		case event_name::CHANNEL_DELETE:				procces_event<event_name::CHANNEL_DELETE>(data); break;;
		case event_name::CHANNEL_PINS_UPDATE:			procces_event<event_name::CHANNEL_PINS_UPDATE>(data); break;;
		case event_name::GUILD_CREATE:					procces_event<event_name::GUILD_CREATE>(data); break;;
		case event_name::GUILD_UPDATE:					procces_event<event_name::GUILD_UPDATE>(data); break;;
		case event_name::GUILD_DELETE:					procces_event<event_name::GUILD_DELETE>(data); break;;
		case event_name::GUILD_BAN_ADD:					procces_event<event_name::GUILD_BAN_ADD>(data); break;;
		case event_name::GUILD_BAN_REMOVE:				procces_event<event_name::GUILD_BAN_REMOVE>(data); break;;
		case event_name::GUILD_EMOJI_UPDATE:			procces_event<event_name::GUILD_EMOJI_UPDATE>(data); break;;
		case event_name::GUILD_INTEGRATIONS_UPDATE:		procces_event<event_name::GUILD_INTEGRATIONS_UPDATE>(data); break;;
		case event_name::GUILD_MEMBER_ADD:				procces_event<event_name::GUILD_MEMBER_ADD>(data); break;;
		case event_name::GUILD_MEMBER_REMOVE:			procces_event<event_name::GUILD_MEMBER_REMOVE>(data); break;;
		case event_name::GUILD_MEMBER_UPDATE:			procces_event<event_name::GUILD_MEMBER_UPDATE>(data); break;;
		case event_name::GUILD_MEMBERS_CHUNK:			procces_event<event_name::GUILD_MEMBERS_CHUNK>(data); break;;
		case event_name::GUILD_ROLE_CREATE:				procces_event<event_name::GUILD_ROLE_CREATE>(data); break;;
		case event_name::GUILD_ROLE_UPDATE:				procces_event<event_name::GUILD_ROLE_UPDATE>(data); break;;
		case event_name::GUILD_ROLE_DELETE:				procces_event<event_name::GUILD_ROLE_DELETE>(data); break;;
		case event_name::MESSAGE_CREATE:				procces_event<event_name::MESSAGE_CREATE>(data); break;;
		case event_name::MESSAGE_UPDATE:				procces_event<event_name::MESSAGE_UPDATE>(data); break;;
		case event_name::MESSAGE_DELETE:				procces_event<event_name::MESSAGE_DELETE>(data); break;;
		case event_name::MESSAGE_DELETE_BULK:			procces_event<event_name::MESSAGE_DELETE_BULK>(data); break;;
		case event_name::MESSAGE_REACTION_ADD:			procces_event<event_name::MESSAGE_REACTION_ADD>(data); break;;
		case event_name::MESSAGE_REACTION_REMOVE:		procces_event<event_name::MESSAGE_REACTION_REMOVE>(data); break;;
		case event_name::MESSAGE_REACTION_REMOVE_ALL:	procces_event<event_name::MESSAGE_REACTION_REMOVE_ALL>(data); break;;
		case event_name::PRESENCE_UPDATE:				procces_event<event_name::PRESENCE_UPDATE>(data); break;;
		case event_name::TYPING_START:					procces_event<event_name::TYPING_START>(data); break;;
		case event_name::USER_UPDATE:					procces_event<event_name::USER_UPDATE>(data); break;;
		case event_name::VOICE_STATE_UPDATE:			procces_event<event_name::VOICE_STATE_UPDATE>(data); break;;
		case event_name::VOICE_SERVER_UPDATE:			procces_event<event_name::VOICE_SERVER_UPDATE>(data); break;;
		case event_name::WEBHOOKS_UPDATE:				procces_event<event_name::WEBHOOKS_UPDATE>(data); break;;

		}
	}catch(...) {
		//swallow
	}
}

void internal_shard::m_opcode1_send_heartbeat() const {
	if (is_disconnected())return;
	std::string t = 
R"({
	"op": 1,
	"d" : null
})";
	m_client->send_thing(std::move(t));
}

void internal_shard::m_opcode2_send_identity() const {
	m_sendIdentity();
}

void internal_shard::m_opcode3_send_presence() const {
	nlohmann::json val;
	val["op"] = 3;
	val["data"] = presence();
	m_client->send_thing(val.dump());
}

std::pair<cerwy::task<nlohmann::json>, cerwy::task<std::string>> internal_shard::m_opcode4(const voice_channel& channel) {
	cerwy::promise<nlohmann::json> name_promise;
	cerwy::promise<std::string> session_id_promise;

	auto name = name_promise.get_task();
	auto session_id_task = session_id_promise.get_task();

	m_things_waiting_for_voice_endpoint.insert(std::make_pair(channel.guild_id(), std::move(name_promise)));
	m_things_waiting_for_voice_endpoint2.insert(std::make_pair(channel.guild_id(), std::move(session_id_promise)));

	m_client->send_thing(
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
)"_format(channel.guild_id().val, channel.id().val));


	return { std::move(name),std::move(session_id_task) };
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
	m_client->send_thing(s.dump());	
}

cerwy::task<void> internal_shard::m_opcode9_on_invalid_session(const nlohmann::json& d) {
	//boost::asio::steady_timer timer(m_ioc, std::chrono::steady_clock::now() + 5s);
	//auto ec = co_await timer.async_wait(cerwy::bind_executor(strand(),use_task_return_ec));

	//remove this for the above later when concepts and modules are in boost asio,
	//cuz it'll too big to compile on vc;-;
	std::this_thread::sleep_for(5s);

	if (d.get<bool>())
		m_opcode6_send_resume();
	else
		m_opcode2_send_identity();

	co_return;
}

void internal_shard::send_heartbeat() {
	if (!m_op11.load()) {
		reconnect();
	}
	m_opcode1_send_heartbeat();
	m_op11 = false;
	
	m_parent->heartbeat_sender.execute(std::make_pair([this](){
		send_heartbeat();
	}, std::chrono::steady_clock::now() + std::chrono::milliseconds(m_hb_interval)));

	/*
	change ^ to v later
	auto t = std::make_unique<boost::asio::steady_timer>(m_ioc);
	t->expires_after(std::chrono::milliseconds(m_hb_interval));
	t->async_wait([this,temp = std::move(t)](auto ec){
		if(!ec)
			send_heartbeat();
	})
	*/
	
}

void internal_shard::m_opcode10_on_hello(nlohmann::json& stuff) {
	m_hb_interval = stuff["heartbeat_interval"].get<int>();
	send_heartbeat();
	m_sendIdentity();
}

void internal_shard::m_opcode11(nlohmann::json& data) {
	m_op11 = true;
}

void internal_shard::reconnect() {
	if (!is_disconnected())
		close_connection(1000);
	//m_parent->reconnect(this, m_shard_number);
}

void internal_shard::send_resume() const {
	std::string temp(R"({
		"op":6,
		"d":{
			"token":)"s + std::string(m_parent->token()) + R"(
			"session_id:)"s + m_session_id + R"(
			"seq:")" + std::to_string(m_seqNum) + R"(
		}
	}
	)"s);
	m_client->send_thing(std::move(temp));
}

void internal_shard::replay_events_for(snowflake guild_id) {
	auto it = m_backed_up_events.find(guild_id);
	if (it != m_backed_up_events.end()) {
		auto backed_up_events = std::move(it->second);
		m_backed_up_events.erase(it);
		for (auto& event : backed_up_events) {
			m_opcode0(event.first, event.second, 0);
		}
	}else {
		//???????????
	}
}

void internal_shard::m_sendIdentity()const {
	/*
	std::string identity = R"({
		"op":2,
		"d":{
			"token":")"s + std::string(m_parent->token()) + R"(",
			"properties":{
				"$os":")"s + s_os + R"(",
				"$browser":"cerwy",
				"$device":"cerwy"
			},
			"compress":false,
			"large_threshold":51,
			"shard":[)"s + std::to_string(m_shard_number) + "," + std::to_string(m_parent->num_shards()) + R"(],	
			"presence":)"s + presence().dump() + 
		R"(,"intents":)"s + std::to_string((uint32_t)m_intents) + 
		"}"s + "}";
	*/
	auto identity2 = fmt::format(R"(
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
)", m_parent->token(), s_os, large_threshold, m_shard_number, m_parent->num_shards(), presence().dump(), m_intents.int_value());
		
	m_client->send_thing(std::move(identity2));
}

template <>
void internal_shard::procces_event<event_name::READY>(nlohmann::json& event) {
	m_trace2 = event["_trace"];
	m_self_user = event["user"].get<user>();
	m_session_id = event["session_id"].get<std::string>();
	//ignore unavailable guilds, only need size
	m_guilds.reserve(event["guilds"].size());

	const auto& private_channels = event["private_channels"].get_ref<const nlohmann::json::array_t&>();
	m_dm_channels.reserve(private_channels.size());
	for (const auto& c : private_channels)
		insert_proj_as_key(m_dm_channels, c.get<dm_channel>(),get_id);
}

template <>
void internal_shard::procces_event<event_name::GUILD_CREATE>(nlohmann::json& data) {
	Guild& guild = m_guilds.insert(std::make_pair(data["id"].get<snowflake>(), data.get<Guild>())).first->second; 	
	if ( m_intents.has_intents(intent::GUILD_MEMBERS) && guild.m_member_count >= large_threshold) {
		guild.m_is_ready = false;		
		request_guild_members(guild);		
	} else {
		//add members to guild
		for (const auto& member_json : data["members"]) {
			auto member = member_json.get<guild_member>();
			member.m_guild = &guild;
			const auto id = member.id();
			guild.m_members.insert(std::make_pair(id, std::move(member)));
		}		
	}

	const auto channels = data["channels"].get_ref<const nlohmann::json::array_t&>();
	guild.m_text_channel_ids.reserve(channels.size());
	guild.m_voice_channel_ids.reserve(std::max(3ull, channels.size() / 10));//random numbers

	for (const auto& channel_json : channels) {
		if (const auto type = channel_json["type"].get<int>(); type == 0) {//text
			auto& channel = m_text_channel_map.insert(std::make_pair(channel_json["id"].get<snowflake>(), channel_json.get<text_channel>())).first->second;
			channel.m_guild_id = guild.id();
			guild.m_text_channel_ids.push_back(channel.id());
			channel.m_guild = &guild;
		} else if (type == 2) {//voice
			auto& channel = m_voice_channel_map.insert(std::make_pair(channel_json["id"].get<snowflake>(), channel_json.get<voice_channel>())).first->second;
			channel.m_guild_id = guild.id();
			guild.m_voice_channel_ids.push_back(channel.id());
			channel.m_guild = &guild;
		} else if (type == 4) {//guild catagory
			auto& channel = m_channel_catagory_map.insert(std::make_pair(channel_json["id"].get<snowflake>(), channel_json.get<channel_catagory>())).first->second;
			channel.m_guild_id = guild.id();
			guild.m_channel_catagorie_ids.push_back(channel.id());
			channel.m_guild = &guild;
		}

		//type == 3 is missing since it's DM channel, dm channels don't exist in guild channels
	}

	for (auto& channel:	guild.m_text_channel_ids |
						ranges::views::transform(hof::map_with(m_text_channel_map)) |
		 				ranges::views::filter(&text_channel::has_parent)) {
		channel.m_parent = &m_channel_catagory_map[channel.m_parent_id];
	}

	for (const auto& channel_id : guild.m_voice_channel_ids) {
		auto& channel = m_voice_channel_map[channel_id];
		if (channel.m_parent_id.val)
			channel.m_parent = &m_channel_catagory_map[channel.m_parent_id];
	}

	if (guild.m_is_ready) {
		for (const auto& presence : data["presences"]) {
			auto& member = guild.m_members[presence["user"]["id"].get<snowflake>()];
			member.m_status = string_to_status(presence["status"].get_ref<const nlohmann::json::string_t&>());
			const auto temp = presence["game"];
			if (!temp.is_null())
				member.m_game.emplace(temp.get<activity>());
		}
	}else {
		guild.m_presences = std::move(data["presences"]);//save it for later
	}

	guild.m_shard = this;	
}

//TODO
//New Properties on Guild Members Chunk Event
//April 24, 2020
//The Guild Members Chunk gateway event now contains two properties : chunk_index and chunk_count.
//These values can be used to keep track of how many events you have left to receive in response to a Request Guild Members command.

template<>
void internal_shard::procces_event<event_name::GUILD_MEMBERS_CHUNK>(nlohmann::json& e) {//really bad ;-;
	std::cout << e.dump(0) << std::endl;
	Guild& g = m_guilds[e["guild_id"].get<snowflake>()];
	for (const auto& i : e["members"]) {
		auto temp_member = i.get<guild_member>();
		const auto id = temp_member.id();
		temp_member.m_guild = &g;
		g.m_members[id] = std::move(temp_member);
	}
	
	auto& chunks_left = m_chunks_left_for_guild[g.id()];
	
	if (chunks_left == 0) {//chunks_left has just been initialized
		chunks_left = e["chunk_count"].get<int>();
	}
	
	if (--chunks_left == 0){
		m_chunks_left_for_guild.erase(g.id());
		g.m_is_ready = true;
		for (const auto& presence : g.m_presences) {
			auto& member = g.m_members[presence["user"]["id"].get<snowflake>()];
			member.m_status = string_to_status(presence["status"].get_ref<const nlohmann::json::string_t&>());
			const auto game = presence["game"];
			if (!game.is_null())
				member.m_game.emplace(game.get<activity>());
		}

		replay_events_for(g.id());
		
		//g.m_presences.clear();
		g.m_presences = 0;//set to int to deallocate memory, .clear keeps the buffer		
	}
}

/*
template<typename T>
struct always_map{
	constexpr always_map(T a):m_thing(std::forward<T>(a)){}

	template<typename A>
	std::add_rvalue_reference_t<T> operator[](A&&) noexcept{
		return m_thing;
	}
	template<typename A>
	std::add_const_t<std::add_rvalue_reference_t<T>> operator[](A&&) const noexcept{
		return m_thing;
	}

	// ReSharper disable CppMemberFunctionMayBeStatic
	size_t size()const noexcept {
		// ReSharper restore CppMemberFunctionMayBeStatic
		return 1;
	}

	struct fake_iterator{
		std::pair<int,std::add_rvalue_reference_t<T>> operator*()const {
			return std::pair<int, std::add_rvalue_reference_t<T>>(0,(*parent)[0]);//0 is random
		}

		arrow_proxy<std::pair<int, std::add_rvalue_reference_t<T>>> operator->()const {
			return { **this };
		}

		bool operator==(fake_iterator o)const noexcept{
			return parent == o.parent;
		}
		bool operator!=(fake_iterator o)const noexcept {
			return parent == o.parent;
		}
		always_map* parent;

	};

	fake_iterator begin() {
		return { this };
	}

	// ReSharper disable once CppMemberFunctionMayBeStatic
	fake_iterator end() const{ //NOLINT
		return { nullptr };
	}

	template<typename A>
	fake_iterator find(A&&) {
		return { this };
	}

private:
	T m_thing;
};
*/

template <>
void internal_shard::procces_event<event_name::MESSAGE_CREATE>(nlohmann::json& e) {
	const auto channel_id = e["channel_id"].get<snowflake>();
	if (auto it = m_text_channel_map.find(channel_id); it != m_text_channel_map.end()) {

		text_channel& ch = it->second;
		auto& guild = ch.guild();
		if(!guild.m_is_ready) {
			m_backed_up_events[guild.id()].emplace_back(std::move(e), event_name::MESSAGE_CREATE);
			return;
		}
		
		auto msg = create_msg<guild_text_message>(ch, e, m_guilds[ch.m_guild_id].m_members);	
		
		m_parent->on_guild_text_msg(std::move(msg), *this);
	}else {//dm msg		
		dm_channel& ch = m_dm_channels[channel_id];
		auto msg = create_msg<dm_message>(ch, e, ch.m_recipients);
		m_parent->on_dm_msg(std::move(msg), *this);
	}
}

template <>
void internal_shard::procces_event<event_name::CHANNEL_CREATE>(nlohmann::json& channel_json) {
	const auto id = channel_json["id"].get<snowflake>();
	const auto type = channel_json["type"].get<int>();
	const auto guild_id = channel_json.value("guild_id",snowflake{});
	if(guild_id.val && !m_guilds[guild_id].m_is_ready) {
		m_backed_up_events[guild_id].emplace_back(std::move(channel_json),  event_name::CHANNEL_CREATE);
		return;
	}
	if (type == 0) {//text
		auto& channel = m_text_channel_map.insert(std::make_pair(id, channel_json.get<text_channel>())).first->second;
		m_guilds[channel.m_guild_id].m_text_channel_ids.push_back(id);
		channel.m_guild = &m_guilds[channel.m_guild_id];

		channel.m_parent = ptr_or_null(m_channel_catagory_map, channel.m_parent_id);
		m_parent->on_guild_text_channel_create(channel,*this);
	}else if (type == 1 || type == 3) {//DM
		auto& channel = m_dm_channels.insert(std::make_pair(id, channel_json.get<dm_channel>())).first->second;
		m_parent->on_dm_channel_create(channel,*this);
	}else if (type == 2) {//voice
		auto& channel = m_voice_channel_map.insert(std::make_pair(id, channel_json.get<voice_channel>())).first->second;
		m_guilds[channel.m_guild_id].m_voice_channel_ids.push_back(id);
		channel.m_guild = &m_guilds[channel.m_guild_id];

		channel.m_parent = ptr_or_null(m_channel_catagory_map, channel.m_parent_id);
		m_parent->on_guild_voice_channel_create(channel,*this);
	}else if (type == 4) {//guild catagory
		auto& channel = m_channel_catagory_map.insert(std::make_pair(id, channel_json.get<channel_catagory>())).first->second;
		m_guilds[channel.m_guild_id].m_channel_catagorie_ids.push_back(id);

		channel.m_guild = &m_guilds[channel.m_guild_id];
		m_parent->on_guild_channel_catagory_create(channel,*this);
	}	
}

template <>
void internal_shard::procces_event<event_name::CHANNEL_DELETE>(nlohmann::json& e) {
	const auto channel_id = e["id"].get<snowflake>();
	const bool is_guild_channel = e.contains("guild_id");
	if (is_guild_channel) {
		const auto guild_id = e["guild_id"].get<snowflake>();
		Guild& guild = m_guilds[guild_id];
		if(!guild.m_is_ready) {
			m_backed_up_events[guild_id].emplace_back(std::move(e), event_name::CHANNEL_DELETE);
			return;
		}
		
		if (erase_first_quick(guild.m_text_channel_ids, channel_id)) {
			auto channel_it = m_text_channel_map.find(channel_id);
			auto channel = std::move(channel_it->second);
			m_text_channel_map.erase(channel_it);
			m_parent->on_text_channel_delete(std::move(channel),*this);
		}else if(erase_first_quick(guild.m_voice_channel_ids, channel_id)) {
			auto channel_it = m_voice_channel_map.find(channel_id);
			auto channel = std::move(channel_it->second);
			m_voice_channel_map.erase(channel_it);
			//m_parent->on_text_channel_delete(std::move(channel), *this);
		}else if(erase_first_quick(guild.m_channel_catagorie_ids, channel_id)) {
			auto channel_it = m_channel_catagory_map.find(channel_id);
			auto channel = std::move(channel_it->second);
			m_channel_catagory_map.erase(channel_it);

			//m_parent->on_text_channel_delete(std::move(channel), *this);
		}else {
			//wat
		}
		
	}
	else {
		auto it2 = m_dm_channels.find(channel_id);
		if (it2 == m_dm_channels.end())	{
			return;//not guild_channel or dm_channel? 
		}
		dm_channel d = std::move(it2->second);
		m_dm_channels.erase(it2);
		m_parent->on_dm_channel_delete(std::move(d), *this);
	}
}

template <>
void internal_shard::procces_event<event_name::GUILD_MEMBER_ADD>(nlohmann::json& e) {
	const auto guild_id = e["guild_id"].get<snowflake>();
	Guild& guild = m_guilds[guild_id];
	if (!guild.m_is_ready) {
		m_backed_up_events[guild_id].emplace_back(std::move(e), event_name::GUILD_MEMBER_ADD);
		return;
	}

	auto member = e.get<guild_member>();	
	const auto id = member.m_id;

	member.m_guild = &guild;	
	
	auto& member_ref = guild.m_members.insert(std::pair{ id,std::move(member) }).first->second;
	member_ref.m_guild = &guild;
	m_parent->on_guild_member_add(member_ref,*this);
}

template <>
void internal_shard::procces_event<event_name::GUILD_MEMBER_REMOVE>(nlohmann::json& e) {
	Guild& guild = m_guilds[e["guild_id"].get<snowflake>()];
	if (!guild.m_is_ready) {
		m_backed_up_events[guild.m_id].emplace_back(std::move(e), event_name::GUILD_MEMBER_ADD);
		return;
	}
	const auto user_id = e["user"]["id"].get<snowflake>();
	
	auto it = guild.m_members.find(user_id);
	--guild.m_member_count;
	//member not in guild for some reason?
	if(it == guild.m_members.end()) {//;-;
		//create fake member
		guild_member member;
		from_json(e["user"], static_cast<user&>(member));
		member.m_guild = &guild;
		m_parent->on_guild_member_remove(member, false,*this);
	}else {		
		auto member = std::move(it->second);
		guild.m_members.erase(it);
		m_parent->on_guild_member_remove(member,true,*this);
	}
}

template<> 
void internal_shard::procces_event<event_name::RESUMED>(nlohmann::json& e){
	m_trace = e["_trace"].get<std::vector<std::string>>();
};

template<>	
void internal_shard::procces_event<event_name::HELLO>(nlohmann::json& json){
	m_trace = json.at("_trace").get<std::vector<std::string>>();
	m_hb_interval = json["heartbeat_interval"].get<int>();
};

template<>	
void internal_shard::procces_event<event_name::INVALID_SESSION>(nlohmann::json&) {
	
};

template<>
void internal_shard::procces_event<event_name::CHANNEL_UPDATE>(nlohmann::json& e){
	const auto channel_id = e["id"].get<snowflake>();
	const auto type = e["type"].get<int>();
	if(type == 0) {
		auto& channel = m_text_channel_map[channel_id];
		channel = e.get<text_channel>();
		channel.m_parent = ptr_or_null(m_channel_catagory_map, channel.m_parent_id);
	}else if(type == 1) {
		m_dm_channels[channel_id] = e.get<dm_channel>();
	}else if(type == 2) {
		auto& channel = m_voice_channel_map[channel_id];
		channel = e.get<voice_channel>();
		channel.m_parent = ptr_or_null(m_channel_catagory_map, channel.m_parent_id);
	}else if(type == 3) {//;-;
		m_dm_channels[channel_id] = e.get<dm_channel>();
	}else if(type == 4) {
		m_channel_catagory_map[channel_id] = e.get<channel_catagory>();
	}
}

template<>	
void internal_shard::procces_event<event_name::CHANNEL_PINS_UPDATE>(nlohmann::json& e){

};

template<>	
void internal_shard::procces_event<event_name::GUILD_UPDATE>(nlohmann::json& e){
	const auto guild_id = e["id"].get<snowflake>();
	Guild& g = m_guilds[guild_id];
	if(!g.m_is_ready) {
		m_backed_up_events[guild_id].emplace_back(std::move(e), event_name::GUILD_UPDATE);
		return;
	}
	from_json(e, static_cast<partial_guild&>(g));
	m_parent->on_guild_update(g, *this);
};

template<>	
void internal_shard::procces_event<event_name::GUILD_DELETE>(nlohmann::json& e){
	const auto unavailable = e["unavailable"].get<bool>();
	try {
		//in case guild_create never came for some reason
		const Guild g = std::move(m_guilds.extract(e["id"].get<snowflake>()).mapped());
		m_parent->on_guild_remove(g, unavailable, *this);
	}catch(...) {
		//swallow		 
	}
}

template<>	
void internal_shard::procces_event<event_name::GUILD_BAN_ADD>(nlohmann::json& e){
	Guild& g = m_guilds[e["id"].get<snowflake>()];
	auto member = e.get<user>();
	m_parent->on_ban_add(g, member, *this);
};

template<>	
void internal_shard::procces_event<event_name::GUILD_BAN_REMOVE>(nlohmann::json& e){
	auto member = e.get<user>();
	Guild& g = m_guilds[member.id()];
	m_parent->on_ban_remove(g, member, *this);
}

template<>
void internal_shard::procces_event<event_name::GUILD_EMOJI_UPDATE>(nlohmann::json& e){
	m_guilds[e["id"].get<snowflake>()].m_emojis = e["emojis"].get<std::vector<emoji>>();
};

template<>
void internal_shard::procces_event<event_name::GUILD_INTEGRATIONS_UPDATE>(nlohmann::json& event){
	//what is this ;-;
};

template<>
void internal_shard::procces_event<event_name::GUILD_MEMBER_UPDATE>(nlohmann::json& e){
	auto& guild = m_guilds[e["guild_id"].get<snowflake>()];
	if(!guild.m_is_ready) {
		m_backed_up_events[guild.m_id].emplace_back(std::move(e), event_name::GUILD_MEMBER_UPDATE);
		return;
	}
	const auto user_ = e["user"];
	const auto user_id = user_["id"].get<snowflake>();
	const auto it = guild.m_members.find(user_id);
	
	if (it == guild.m_members.end()) {//;-; create guild_member and add it to m_members
		guild_member member;
		from_json(user_, static_cast<user&>(member));
		member.m_roles = e["roles"].get<std::vector<snowflake>>();		
		if(const auto it2 = e.find("nick"); it2 != e.end())
			member.m_nick = e["nick"].is_null() ? "" : e["nick"].get<std::string>();
		member.m_guild = &guild;
		const auto id = member.id();
		auto& member_in_map = guild.m_members.insert(std::pair(id,std::move(member))).first->second;
		m_parent->on_guild_member_update(member_in_map, *this);
	}else{
		auto& member = (*it).second;
		from_json(user_, static_cast<user&>(member));
		member.m_roles = e["roles"].get<std::vector<snowflake>>();
		if(const auto it2 = e.find("nick"); it2 != e.end())
			member.m_nick = e["nick"].is_null()? member.m_nick : e["nick"].get<std::string>();
		m_parent->on_guild_member_update(member, *this);
	}
	
};

template<>
void internal_shard::procces_event<event_name::GUILD_ROLE_CREATE>(nlohmann::json& e){
	Guild& guild = m_guilds[e["guild_id"].get<snowflake>()];
	m_parent->on_role_create(guild, guild.m_roles.insert(std::make_pair(e["role"]["id"].get<snowflake>(),e["role"].get<guild_role>())).first->second, *this);
};

template<>	
void internal_shard::procces_event<event_name::GUILD_ROLE_UPDATE>(nlohmann::json& e){
	Guild& guild = m_guilds[e["guild_id"].get<snowflake>()];
	auto new_role = e["role"].get<guild_role>();
	guild_role& old_role = guild.m_roles[new_role.id()];
	std::swap(old_role, new_role);
	//old_role is now new_role
	m_parent->on_role_update(guild,new_role,old_role , *this);	
};

template<>	
void internal_shard::procces_event<event_name::GUILD_ROLE_DELETE>(nlohmann::json& e){
	Guild& guild = m_guilds[e["guild_id"].get<snowflake>()];
	const auto role_id = e["role_id"].get<snowflake>();	
	const guild_role old_role = std::move(guild.m_roles.extract(role_id).mapped());
	for (auto& member : guild.mutable_members_list()) {		
		erase_first_quick(member.m_roles, old_role.id());
	}	
	m_parent->on_role_delete(guild, old_role, *this);
};

template<>	
void internal_shard::procces_event<event_name::MESSAGE_UPDATE>(nlohmann::json& e){
	const auto channel_id = e["channel_id"].get<snowflake>();
	if (auto it = m_text_channel_map.find(channel_id); it != m_text_channel_map.end()) {//guild text msg	
		text_channel& ch = it->second;
		auto& guild = m_guilds[ch.m_guild_id];
		if(!guild.m_is_ready) {
			m_backed_up_events[guild.m_id].emplace_back(std::move(e), event_name::MESSAGE_UPDATE);
			return;
		}

		auto msg = createMsgUpdate<guild_msg_update>(ch, e, guild.m_members);
		//auto it2 = ranges::find_if(ch.m_msg_cache.data(), id_equal_to(msg.id()));

		/*
		if (it2 != ch.msg_cache().end()) {
			guild_text_message old_msg = *it2;
			it2->m_content = e.value("content", it2->m_content);
			it2->m_reactions = e.value("reaction", it2->m_reactions);
			m_parent->on_guild_msg_update(msg, old_msg, *this);
		}else {
		}
		*/
			m_parent->on_guild_msg_update(std::move(msg), std::nullopt, *this);
	}else {//dm msg
		dm_channel& ch = m_dm_channels[channel_id];
		auto msg = createMsgUpdate<dm_msg_update>(ch, e, ch.m_recipients);
		/*auto it2 = ranges::find_if(ch.m_msg_cache.data(), id_equal_to(msg.id()));

		if (it2 != ch.msg_cache().end()) {
			dm_message old_msg = *it2;
			it2->m_content = e.value("content", it2->m_content);
			it2->m_reactions = e.value("reaction", it2->m_reactions);
			m_parent->on_dm_msg_update(msg, old_msg, *this);
		}else {
		}*/
		m_parent->on_dm_msg_update(std::move(msg), std::nullopt, *this);
	}
}

template<>	void internal_shard::procces_event<event_name::MESSAGE_DELETE>(nlohmann::json& e){
	const auto channel_id = e["channel_id"].get<snowflake>();
	const auto msg_id = e["id"].get<snowflake>();
	if (auto it = m_text_channel_map.find(channel_id); it != m_text_channel_map.end()) {//guild text msg
		text_channel& ch = it->second;
		/*auto it2 = ranges::find_if(ch.m_msg_cache.data(), id_equal_to(msg_id));
		if (it2 != ch.msg_cache().end()) {
			m_parent->on_guild_msg_delete(std::move(*it2), channel_id, *this);
		}else {
		}*/
		m_parent->on_guild_msg_delete(std::nullopt, channel_id, *this);
	}else {//dm msg
		dm_channel& ch = m_dm_channels[channel_id];
		/*auto it2 = ranges::find_if(ch.m_msg_cache.data(), id_equal_to(msg_id));
		if (it2 != ch.msg_cache().end()) {
			m_parent->on_dm_msg_delete(std::move(*it2), channel_id, *this);
		}else {
		}*/
		m_parent->on_dm_msg_delete(std::nullopt, channel_id, *this);
	}
};

template<>
void internal_shard::procces_event<event_name::MESSAGE_DELETE_BULK>(nlohmann::json& e){
	try{
		m_parent->on_message_bulk(e["ids"].get<std::vector<snowflake>>(),m_text_channel_map.at(e["channel_id"].get<snowflake>()),*this);
	}catch (...) {//;-;		
		m_parent->on_dm_message_bulk(e["ids"].get<std::vector<snowflake>>(), m_dm_channels[e["channel_id"].get<snowflake>()], *this);
	}
};

template<>	
void internal_shard::procces_event<event_name::MESSAGE_REACTION_ADD>(nlohmann::json& e){
	const auto channel_id = e["channel_id"].get<snowflake>();
	auto it = m_text_channel_map.find(channel_id);
	auto emojiy = e["emoji"].get<partial_emoji>();

	if (it != m_text_channel_map.end()) {
		auto& channel = it->second;
		auto& member = channel.m_guild->m_members[e["user_id"].get<snowflake>()];
		/*auto msg_it = ranges::find_if(channel.m_msg_cache.data(), id_equal_to(e["message_id"].get<snowflake>()));
		if (msg_it != channel.msg_cache().end()) {
			m_parent->on_guild_reaction_add(
				member, 
				channel,
				*msg_it, 
				add_reaction(msg_it->m_reactions, emojiy, e["user_id"].get<snowflake>(), self_user().id()), 
				*this
			);
		}else {
			//create fake reaction
		}*/
			reaction temp;
			temp.m_count = 1;
			temp.m_emoji = std::move(emojiy);
			temp.m_me = e["user_id"].get<snowflake>() == self_user().id();
			m_parent->on_guild_reaction_add(member, channel, std::nullopt, temp, *this);
	}else {
		auto& channel = m_dm_channels[channel_id];
		auto& person = channel.m_recipients[e["user_id"].get<snowflake>()];
		/*auto msg_it = ranges::find_if(channel.m_msg_cache.data(), id_equal_to(e["message_id"].get<snowflake>()));
		if(msg_it != channel.msg_cache().end()) {
			m_parent->on_dm_reaction_add(
				person, 
				channel, 
				*msg_it, 
				add_reaction(msg_it->m_reactions, emojiy, e["user_id"].get<snowflake>(), self_user().id()), 
				*this
			);
		}
		else {
		}*/
			reaction temp;
			temp.m_count = 1;
			temp.m_emoji = std::move(emojiy);
			temp.m_me = e["user_id"].get<snowflake>() == self_user().id();
			m_parent->on_dm_reaction_add(person, channel, std::nullopt, temp, *this);

	}
};

template<>	
void internal_shard::procces_event<event_name::MESSAGE_REACTION_REMOVE>(nlohmann::json& e){
	const auto channel_id = e["channel_id"].get<snowflake>();
	auto it = m_text_channel_map.find(channel_id);
	auto emojiy = e["emoji"].get<partial_emoji>();
	if (it != m_text_channel_map.end()) {
		auto& channel = it->second;
		auto& member = channel.m_guild->m_members[e["user_id"].get<snowflake>()];
		/*auto msg_it = ranges::find_if(channel.m_msg_cache.data(), id_equal_to(e["message_id"].get<snowflake>()));
		
		if (msg_it != channel.msg_cache().end()) {
			//message is still in msg_cache
			m_parent->on_guild_reaction_add(
				member, 
				channel,
				*msg_it, 
				remove_reaction(msg_it->m_reactions, emojiy, e["user_id"].get<snowflake>(), self_user().id()), 
				*this
			);
		}else {
			//message has been removed from msg_cache, create temp_reaction, reaction.count is no longer accurate
		}*/
			reaction temp;
			temp.m_count = 0;
			temp.m_emoji = std::move(emojiy);
			temp.m_me = false;
			m_parent->on_guild_reaction_add(member, channel, std::nullopt, temp, *this);
	}else {
		auto& channel = m_dm_channels[channel_id];
		auto& person = channel.m_recipients[e["user_id"].get<snowflake>()];
		
		/*auto msg_it = ranges::find_if(channel.m_msg_cache.data(), id_equal_to(e["message_id"].get<snowflake>()));

		if (msg_it != channel.msg_cache().end()) {
			m_parent->on_dm_reaction_add(person, channel, *msg_it, remove_reaction(msg_it->m_reactions, emojiy, e["user_id"].get<snowflake>(), self_user().id()), *this);
		}else {
			
		}*/
		reaction temp;
		temp.m_count = 0;
		temp.m_emoji = std::move(emojiy);
		temp.m_me = false;
		m_parent->on_dm_reaction_add(person, channel, std::nullopt, temp, *this);
	}

};

template<>
void internal_shard::procces_event<event_name::MESSAGE_REACTION_REMOVE_ALL>(nlohmann::json& e){
	auto it = m_text_channel_map.find(e["channel_id"].get<snowflake>());
	if(it == m_text_channel_map.end()) {
		auto& channel = it->second;
		/*auto msg_it = ranges::find_if(channel.m_msg_cache.data(), id_equal_to(e["message_id"].get<snowflake>()));
		if(msg_it == channel.msg_cache().end()) {
		}else {
			msg_it->m_reactions.clear();
			m_parent->on_guild_reaction_remove_all(channel, *msg_it, *this);
		}*/
		m_parent->on_guild_reaction_remove_all(channel, std::nullopt, *this);

	}else {
		auto& channel = m_dm_channels[e["channel_id"].get<snowflake>()];
		/*auto msg_it = ranges::find_if(channel.m_msg_cache.data(), id_equal_to(e["message_id"].get<snowflake>()));
		if (msg_it == channel.msg_cache().end()) {
		}else {
			msg_it->m_reactions.clear();
			m_parent->on_dm_reaction_remove_all(channel, *msg_it, *this);
		}*/
		m_parent->on_dm_reaction_remove_all(channel, std::nullopt, *this);
	}

};

template<>
void internal_shard::procces_event<event_name::PRESENCE_UPDATE>(nlohmann::json& e){
	auto& guild = m_guilds[e["guild_id"].get<snowflake>()];
	if(!guild.m_is_ready) {
		m_backed_up_events[guild.id()].emplace_back(std::move(e), event_name::PRESENCE_UPDATE);
		return;
	}

	const auto id = e["user"]["id"].get<snowflake>();
	auto id_it = guild.m_members.find(id);
	if (id_it == guild.m_members.end())
		return;
	
	auto& user = id_it->second;
	if (const auto status_it = e.find("status"); status_it != e.end())
		user.m_status = string_to_status(status_it->get<std::string>());

	user.m_roles = e.value("roles", user.m_roles);
	if (const auto it = e.find("game"); it != e.end()) {
		const auto game = *it;
		if (!game.is_null())
			user.m_game.emplace(game.get<activity>());
		else
			user.m_game = std::nullopt;
	}
	m_parent->on_presence_update(user, *this);
};

template<>	
void internal_shard::procces_event<event_name::TYPING_START>(nlohmann::json& e){
	const auto channel_id = e["channel_id"].get<snowflake>();
	const auto user_id = e["user_id"].get<snowflake>();
	const auto it = m_text_channel_map.find(channel_id);	
	if (it != m_text_channel_map.end()) {
		Guild& guild = *(it->second.m_guild);
		if (!guild.m_is_ready) {
			m_backed_up_events[guild.id()].emplace_back(std::move(e), event_name::TYPING_START);
			return;
		}
		auto& member = guild.m_members[user_id];
		m_parent->on_guild_typing_start(member, it->second, *this);
	}else {
		dm_channel& channel = m_dm_channels[channel_id];
		auto& member = channel.m_recipients[user_id];
		m_parent->on_dm_typing_start(member, channel, *this);
	}
};

template<>	
void internal_shard::procces_event<event_name::USER_UPDATE>(nlohmann::json& e){
	m_self_user = e.get<user>();
};

template<>	
void internal_shard::procces_event<event_name::VOICE_STATE_UPDATE>(nlohmann::json& e){
	const auto guild_id = e.value("guild_id",snowflake());

	Guild& guild = guild_id.val ? m_guilds[guild_id] : *m_voice_channel_map[e["channel_id"].get<snowflake>()].m_guild;

	const auto user_id = e.value("user_id",self_user().id());

	if(user_id == self_user().id()) {
		auto it = m_things_waiting_for_voice_endpoint2.find(guild.id());
		if (it != m_things_waiting_for_voice_endpoint2.end()) {
			it->second.set_value(e["session_id"].get<std::string>());
		}
		//m_things_waiting_for_voice_endpoint2[guild.id()].set_value(e["session_id"].get<std::string>());
	}

	const auto it = ranges::find_if(guild.m_voice_states, [&](const auto& a) {return a.user_id() == user_id; });
	if(it != guild.m_voice_states.end()) {
		//update the state
		//contains no channel_id => connect to no channel=>disconected
		if(e.contains("channel_id")) //wat does this do ;-;
			*it = e.get<voice_state>();//update it
		else 
			guild.m_voice_states.erase(it);		
	}else {
		//new voice_state
		guild.m_voice_states.push_back(e.get<voice_state>());
	}
};

template<>
void internal_shard::procces_event<event_name::VOICE_SERVER_UPDATE>(nlohmann::json& json){
	const auto guild_id = json["guild_id"].get<snowflake>();
	m_things_waiting_for_voice_endpoint[guild_id].set_value(std::move(json));
};

template<>
void internal_shard::procces_event<event_name::WEBHOOKS_UPDATE>(nlohmann::json&) {
	
}

// ReSharper disable once CppMemberFunctionMayBeConst
void internal_shard::update_presence(const Status s, std::string g) {// NOLINT
	m_status = s;
	m_game_name = std::move(g);
	m_opcode3_send_presence();
}

