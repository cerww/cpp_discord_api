#include "shard.h"
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

template<typename rng, typename U>
[[maybe_unused]]
constexpr bool erase_first_quick(rng&& range, U&& val) {
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

shard::shard(int shard_number, client* t_parent, boost::asio::io_context& ioc, std::string_view gateway) :
	m_shard_number(shard_number),
	m_parent(t_parent),
	m_http_connection(t_parent, ioc),
	m_ioc(ioc),
	m_resolver(ioc),
	m_strand(ioc),
	m_socket(ioc,m_ssl_ctx)
{
	init_shard(shard_number, *this, ioc, gateway);	
}

nlohmann::json shard::presence()const {
	nlohmann::json retVal;
	retVal["since"];
	retVal["game"]["name"] = m_game_name;
	retVal["game"]["type"] = 0;
	retVal["status"] = enum_to_string(m_status);
	retVal["afk"] = m_status == Status::idle;

	return retVal;
}

cerwy::task<voice_connection> shard::connect_voice(const voice_channel& channel) {
	const auto channel_id = channel.id();
	const auto guild_id = channel.guild_id();
	auto [endpoint, session_id_task] = m_opcode4(channel);
	auto ep_json = co_await endpoint;
	auto session_id = co_await session_id_task;
	std::cout << ep_json.dump(1);
	std::string gateway = "wss://"s +  ep_json["endpoint"].get<std::string>() + "/?v=3"s;
	std::cout << gateway << std::endl;

	m_things_waiting_for_voice_endpoint2.erase(channel.guild_id());
	m_things_waiting_for_voice_endpoint.erase(channel.guild_id());
	int i = 0;
	co_return co_await voice_connect_impl(*this, channel, std::move(gateway), ep_json["token"].get<std::string>(), std::move(session_id));
}

cerwy::task<boost::beast::error_code> shard::connect_http_connection() {
	auto ec = co_await m_http_connection.async_connect();
	int tries = 1;
	while(ec && tries <10) {
		ec = co_await m_http_connection.async_connect();
		++tries;
	}
	co_return ec;
}

void shard::doStuff(nlohmann::json stuffs,int op) {	
	//fmt::print("{}\n", stuffs.dump(1));
	switch (op) {
	case 0:
		m_opcode0(std::move(stuffs["d"]), to_event_name(stuffs["t"]), stuffs["s"]);
		break;
	case 1:
		m_opcode1();
		break;
	case 2:break;
	case 3:break;
	case 4:break;
	case 5:break;
	case 6:break;
	case 7:
		m_opcode7();
		break;
	case 8:break;
	case 9:
		m_opcode9(stuffs["d"]);
		break;
	case 10:
		m_opcode10(stuffs["d"]);
		break;
	case 11:
		m_opcode11(stuffs["d"]);
		break;
	default:break;
	}
}

void shard::on_reconnect() {
	send_resume();
}

void shard::rate_limit(std::chrono::system_clock::time_point tp) {
	m_http_connection.sleep_till(tp);
}

void shard::close_connection(int code) {
	m_is_disconnected = true;
	m_client->close(boost::beast::websocket::close_reason(code));
}

void shard::set_up_request(boost::beast::http::request<boost::beast::http::string_body>& req) const{
	m_parent->set_up_request(req);
}

void shard::request_guild_members(Guild& g) const {
	m_opcode8(g.id());
	g.m_members.clear();
	g.m_members.reserve(g.m_member_count);
}

void shard::m_opcode0(nlohmann::json data, event_name event, size_t s) {
	m_seqNum = std::max(s, m_seqNum);
	std::cout << m_seqNum << std::endl;
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

void shard::m_opcode1() const {
	if (is_disconnected())return;
	std::string t = 
R"({
	"op": 1,
	"d" : null
})";
	m_client->send_thing(std::move(t));
}

void shard::m_opcode2() const {
	m_sendIdentity();
}

void shard::m_opcode3() const {
	nlohmann::json val;
	val["op"] = 3;
	val["data"] = presence();
	m_client->send_thing(val.dump());
}

std::pair<cerwy::task<nlohmann::json>, cerwy::task<std::string>> shard::m_opcode4(const voice_channel& channel) {
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

void shard::m_opcode6() const {
	send_resume();
}

void shard::m_opcode7() {
	reconnect();
}

void shard::m_opcode8(snowflake id) const {	
	nlohmann::json s;
	s["op"] = 8;
	s["d"]["guild_id"] = std::to_string(id.val);
	s["d"]["query"] = "";
	s["d"]["limit"] = 0;
	m_client->send_thing(s.dump());
}

cerwy::task<void> shard::m_opcode9(const nlohmann::json& d) {
	//boost::asio::steady_timer timer(m_ioc, std::chrono::steady_clock::now() + 5s);
	//auto ec = co_await timer.async_wait(cerwy::bind_executor(strand(),use_task_return_ec));

	//remove this for ^ later when concepts are in cuz it'll too big to compile ;-;
	std::this_thread::sleep_for(5s);

	if (d.get<bool>())
		m_opcode6();
	else
		m_opcode2();

	co_return;
}

void shard::send_heartbeat() {
	if (!m_op11.load()) {
		reconnect();
	}
	m_opcode1();
	m_op11 = false;
	m_parent->heartbeat_sender.execute(std::make_pair([this](){
		send_heartbeat();
	}, std::chrono::steady_clock::now() + std::chrono::milliseconds(m_hb_interval)));
}

void shard::m_opcode10(nlohmann::json& stuff) {
	m_hb_interval = stuff["heartbeat_interval"].get<int>();
	send_heartbeat();
	//m_opcode1();
	m_sendIdentity();
}

void shard::m_opcode11(nlohmann::json& data) {
	m_op11 = true;
}

void shard::reconnect() {
	if (!is_disconnected())
		close_connection(1000);
	//m_parent->reconnect(this, m_shard_number);
}

void shard::send_resume() const {
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

void shard::replay_events_for(snowflake guild_id) {
	auto it = m_backed_up_events.find(guild_id);
	if (it != m_backed_up_events.end()) {
		auto backed_up_events = std::move(it->second);
		m_backed_up_events.erase(it);
		for (auto& event : backed_up_events) {
			m_opcode0(event.first, event.second, 0);
		}
	}
}

void shard::m_sendIdentity()const {
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
		"}"s + "}";

	m_client->send_thing(std::move(identity));
}

template <>
void shard::procces_event<event_name::READY>(nlohmann::json& event) {
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
void shard::procces_event<event_name::GUILD_CREATE>(nlohmann::json& data) {
	Guild& guild = m_guilds.insert(std::make_pair(data["id"].get<snowflake>(), data.get<Guild>())).first->second; 	
	if (guild.m_member_count >= large_threshold) {
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
	guild.m_text_channels.reserve(channels.size());
	guild.m_voice_channels.reserve(std::max(3ull, channels.size() / 10));//random number

	for (const auto& item : channels) {
		if (const auto type = item["type"].get<int>(); type == 0) {//text
			auto& channel = m_text_channel_map.insert(std::make_pair(item["id"].get<snowflake>(), item.get<text_channel>())).first->second;
			channel.m_guild_id = guild.id();
			guild.m_text_channels.push_back(channel.id());
			channel.m_guild = &guild;
		} else if (type == 2) {//voice
			auto& channel = m_voice_channel_map.insert(std::make_pair(item["id"].get<snowflake>(), item.get<voice_channel>())).first->second;
			channel.m_guild_id = guild.id();
			guild.m_voice_channels.push_back(channel.id());
			channel.m_guild = &guild;
		} else if (type == 4) {//guild catagory
			auto& channel = m_channel_catagory_map.insert(std::make_pair(item["id"].get<snowflake>(), item.get<channel_catagory>())).first->second;
			channel.m_guild_id = guild.id();
			guild.m_channel_catagories.push_back(channel.id());
			channel.m_guild = &guild;
		}
	}

	for (auto& channel:	guild.m_text_channels |
						ranges::view::transform(hof::map_with(m_text_channel_map)) |
		 				ranges::view::filter(&text_channel::has_parent)) {
		channel.m_parent = &m_channel_catagory_map[channel.m_parent_id];
	}

	for (const auto& channel_id : guild.m_voice_channels) {
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
	//auto qaudijhsadff = data.get<std::optional<int>>();

	guild.m_shard = this;	
}

template<>
void shard::procces_event<event_name::GUILD_MEMBERS_CHUNK>(nlohmann::json& e) {//really bad ;-;
	Guild& g = m_guilds[e["guild_id"].get<snowflake>()];
	for (const auto& i : e["members"]) {
		auto temp_member = i.get<guild_member>();
		const auto id = temp_member.id();
		temp_member.m_guild = &g;
		g.m_members[id] = std::move(temp_member);
	}

	if (g.m_members.size() == g.m_member_count) {// it's done sending stuff 
		g.m_is_ready = true;
		for (const auto& presence : g.m_presences) {
			auto& member = g.m_members[presence["user"]["id"].get<snowflake>()];
			member.m_status = string_to_status(presence["status"].get_ref<const nlohmann::json::string_t&>());
			const auto temp = presence["game"];
			if (!temp.is_null())
				member.m_game.emplace(temp.get<activity>());
		}

		replay_events_for(g.id());
		
		//g.m_presences.clear();
		g.m_presences = 0;//to deallocate memory, .clear keeps the buffer
	}
}

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

template <>
void shard::procces_event<event_name::MESSAGE_CREATE>(nlohmann::json& e) {
	const auto channel_id = e["channel_id"].get<snowflake>();
	if (auto it = m_text_channel_map.find(channel_id); it != m_text_channel_map.end()) {

		text_channel& ch = it->second;
		auto msg = [&]()->guild_text_message {//iife
			if(!e["mentions"].empty())
				return create_msg<guild_text_message>(ch, e, m_guilds[ch.m_guild_id].m_members);

			auto& member = m_guilds[ch.m_guild_id].m_members[e["author"]["id"].get<snowflake>()];
			return create_msg<guild_text_message>(ch, e, always_map<guild_member&>(member));
		}();
		
		m_parent->on_guild_text_msg(ch.p_add_msg(std::move(msg)), *this);
	}else {//dm msg		
		dm_channel& ch = m_dm_channels[channel_id];
		auto msg = create_msg<dm_message>(ch, e, ch.m_recipients);
		m_parent->on_dm_msg(ch.m_add_msg(std::move(msg)), *this);
	}
}

template <>
void shard::procces_event<event_name::CHANNEL_CREATE>(nlohmann::json& channel_json) {	
	const auto id = channel_json["id"].get<snowflake>();
	const auto type = channel_json["type"].get<int>();
	const auto guild_id = channel_json.value("guild_id",snowflake{});
	if(guild_id.val && !m_guilds[guild_id].m_is_ready) {
		m_backed_up_events[guild_id].emplace_back(std::move(channel_json),  event_name::CHANNEL_CREATE);
		return;
	}
	if (type == 0) {//text
		auto& channel = m_text_channel_map.insert(std::make_pair(id, channel_json.get<text_channel>())).first->second;
		m_guilds[channel.m_guild_id].m_text_channels.push_back(id);
		channel.m_guild = &m_guilds[channel.m_guild_id];

		channel.m_parent = ptr_or_null(m_channel_catagory_map, channel.m_parent_id);
		m_parent->on_guild_text_channel_create(channel,*this);
	}else if (type == 1 || type == 3) {//DM
		auto& channel = m_dm_channels.insert(std::make_pair(id, channel_json.get<dm_channel>())).first->second;
		m_parent->on_dm_channel_create(channel,*this);
	}else if (type == 2) {//voice
		auto& channel = m_voice_channel_map.insert(std::make_pair(id, channel_json.get<voice_channel>())).first->second;
		m_guilds[channel.m_guild_id].m_voice_channels.push_back(id);
		channel.m_guild = &m_guilds[channel.m_guild_id];

		channel.m_parent = ptr_or_null(m_channel_catagory_map, channel.m_parent_id);
		m_parent->on_guild_voice_channel_create(channel,*this);
	}else if (type == 4) {//guild catagory
		auto& channel = m_channel_catagory_map.insert(std::make_pair(id, channel_json.get<channel_catagory>())).first->second;
		m_guilds[channel.m_guild_id].m_channel_catagories.push_back(id);

		channel.m_guild = &m_guilds[channel.m_guild_id];
		m_parent->on_guild_channel_catagory_create(channel,*this);
	}	
}

template <>
void shard::procces_event<event_name::CHANNEL_DELETE>(nlohmann::json& e) {
	const auto channel_id = e["id"].get<snowflake>();
	if (auto it = m_text_channel_map.find(channel_id); it != m_text_channel_map.end()) {
		text_channel channel = std::move(it->second);
		m_text_channel_map.erase(it);
		Guild& g = m_guilds[channel.guild_id()];
		erase_first_quick(g.m_text_channels, channel_id);
		m_parent->on_text_channel_delete(channel, *this);
	}else {
		auto it2 = m_dm_channels.find(channel_id);
		if (it2 == m_dm_channels.end())	{
			return;
		}
		dm_channel d = std::move(it2->second);
		m_dm_channels.erase(it2);
		m_parent->on_dm_channel_delete(d, *this);
	}
}

template <>
void shard::procces_event<event_name::GUILD_MEMBER_ADD>(nlohmann::json& e) {	
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
void shard::procces_event<event_name::GUILD_MEMBER_REMOVE>(nlohmann::json& e) {
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
void shard::procces_event<event_name::RESUMED>(nlohmann::json& e){
	m_trace = e["_trace"].get<std::vector<std::string>>();
};

template<>	
void shard::procces_event<event_name::HELLO>(nlohmann::json& json){
	m_trace = json.at("_trace").get<std::vector<std::string>>();
	m_hb_interval = json["heartbeat_interval"].get<int>();
};

template<>	
void shard::procces_event<event_name::INVALID_SESSION>(nlohmann::json&) {
	
};

template<>
void shard::procces_event<event_name::CHANNEL_UPDATE>(nlohmann::json& e){
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
void shard::procces_event<event_name::CHANNEL_PINS_UPDATE>(nlohmann::json& e){

};

template<>	
void shard::procces_event<event_name::GUILD_UPDATE>(nlohmann::json& e){
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
void shard::procces_event<event_name::GUILD_DELETE>(nlohmann::json& e){
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
void shard::procces_event<event_name::GUILD_BAN_ADD>(nlohmann::json& e){
	Guild& g = m_guilds[e["id"].get<snowflake>()];
	auto member = e.get<user>();
	m_parent->on_ban_add(g, member, *this);
};

template<>	
void shard::procces_event<event_name::GUILD_BAN_REMOVE>(nlohmann::json& e){
	auto member = e.get<user>();
	Guild& g = m_guilds[member.id()];
	m_parent->on_ban_remove(g, member, *this);
}

template<>
void shard::procces_event<event_name::GUILD_EMOJI_UPDATE>(nlohmann::json& e){
	m_guilds[e["id"].get<snowflake>()].m_emojis = e["emojis"].get<std::vector<emoji>>();
};

template<>
void shard::procces_event<event_name::GUILD_INTEGRATIONS_UPDATE>(nlohmann::json&){
	//what is this ;-;
};

template<>
void shard::procces_event<event_name::GUILD_MEMBER_UPDATE>(nlohmann::json& e){
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
void shard::procces_event<event_name::GUILD_ROLE_CREATE>(nlohmann::json& e){
	Guild& guild = m_guilds[e["guild_id"].get<snowflake>()];
	m_parent->on_role_create(guild, guild.m_roles.insert(std::make_pair(e["role"]["id"].get<snowflake>(),e["role"].get<guild_role>())).first->second, *this);
};

template<>	
void shard::procces_event<event_name::GUILD_ROLE_UPDATE>(nlohmann::json& e){	
	Guild& guild = m_guilds[e["guild_id"].get<snowflake>()];
	auto new_role = e["role"].get<guild_role>();
	guild_role& old_role = guild.m_roles[new_role.id()];
	std::swap(old_role, new_role);
	//old_role is now new_role
	m_parent->on_role_update(guild,new_role,old_role , *this);	
};

template<>	
void shard::procces_event<event_name::GUILD_ROLE_DELETE>(nlohmann::json& e){
	Guild& guild = m_guilds[e["guild_id"].get<snowflake>()];
	const auto role_id = e["role_id"].get<snowflake>();	
	const guild_role old_role = std::move(guild.m_roles.extract(role_id).mapped());
	for (auto& member : guild.mutable_members_list()) {		
		erase_first_quick(member.m_roles, old_role.id());
	}
	m_parent->on_role_delete(guild, old_role, *this);
};

template<>	
void shard::procces_event<event_name::MESSAGE_UPDATE>(nlohmann::json& e){
	const auto channel_id = e["channel_id"].get<snowflake>();
	if (auto it = m_text_channel_map.find(channel_id); it != m_text_channel_map.end()) {//guild text msg	
		text_channel& ch = it->second;
		auto& guild = m_guilds[ch.m_guild_id];
		if(!guild.m_is_ready) {
			m_backed_up_events[guild.m_id].emplace_back(std::move(e), event_name::MESSAGE_UPDATE);
			return;
		}

		auto msg = createMsgUpdate<guild_msg_update>(ch, e, guild.m_members);
		auto it2 = ranges::find_if(ch.m_msg_cache.data(), id_equal_to(msg.id()));

		if (it2 != ch.msg_cache().end()) {
			guild_text_message old_msg = *it2;
			it2->m_content = e.value("content", it2->m_content);
			it2->m_reactions = e.value("reaction", it2->m_reactions);
			m_parent->on_guild_msg_update(msg, old_msg, *this);
		}else {
			m_parent->on_guild_msg_update(msg, std::nullopt, *this);
		}
	}else {//dm msg
		dm_channel& ch = m_dm_channels[channel_id];
		auto msg = createMsgUpdate<dm_msg_update>(ch, e, ch.m_recipients);
		auto it2 = ranges::find_if(ch.m_msg_cache.data(), id_equal_to(msg.id()));

		if (it2 != ch.msg_cache().end()) {
			dm_message old_msg = *it2;
			it2->m_content = e.value("content", it2->m_content);
			it2->m_reactions = e.value("reaction", it2->m_reactions);
			m_parent->on_dm_msg_update(msg, old_msg, *this);
		}else {
			m_parent->on_dm_msg_update(msg, std::nullopt, *this);
		}
	}
}

template<>	void shard::procces_event<event_name::MESSAGE_DELETE>(nlohmann::json& e){
	const auto channel_id = e["channel_id"].get<snowflake>();
	const auto msg_id = e["id"].get<snowflake>();
	if (auto it = m_text_channel_map.find(channel_id); it != m_text_channel_map.end()) {//guild text msg
		text_channel& ch = it->second;
		auto it2 = ranges::find_if(ch.m_msg_cache.data(), id_equal_to(msg_id));
		if (it2 != ch.msg_cache().end()) {
			m_parent->on_guild_msg_delete(std::move(*it2), channel_id, *this);
		}else {
			m_parent->on_guild_msg_delete(std::nullopt, channel_id, *this);
		}
	}else {//dm msg
		dm_channel& ch = m_dm_channels[channel_id];
		auto it2 = ranges::find_if(ch.m_msg_cache.data(), id_equal_to(msg_id));
		if (it2 != ch.msg_cache().end()) {
			m_parent->on_dm_msg_delete(std::move(*it2), channel_id, *this);
		}else {
			m_parent->on_dm_msg_delete(std::nullopt, channel_id, *this);
		}
	}
};

template<>
void shard::procces_event<event_name::MESSAGE_DELETE_BULK>(nlohmann::json& e){
	try{
		m_parent->on_message_bulk(e["ids"].get<std::vector<snowflake>>(),m_text_channel_map.at(e["channel_id"].get<snowflake>()),*this);
	}catch (...) {//;-;		
		m_parent->on_dm_message_bulk(e["ids"].get<std::vector<snowflake>>(), m_dm_channels[e["channel_id"].get<snowflake>()], *this);
	}
};

template<>	
void shard::procces_event<event_name::MESSAGE_REACTION_ADD>(nlohmann::json& e){
	const auto channel_id = e["channel_id"].get<snowflake>();
	auto it = m_text_channel_map.find(channel_id);
	auto emojiy = e["emoji"].get<partial_emoji>();

	if (it != m_text_channel_map.end()) {
		auto& channel = it->second;
		auto& member = channel.m_guild->m_members[e["user_id"].get<snowflake>()];
		auto msg_it = ranges::find_if(channel.m_msg_cache.data(), id_equal_to(e["message_id"].get<snowflake>()));
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
			reaction temp;
			temp.m_count = 1;
			temp.m_emoji = std::move(emojiy);
			temp.m_me = e["user_id"].get<snowflake>() == self_user().id();
			m_parent->on_guild_reaction_add(member, channel, std::nullopt, temp, *this);
		}
	}else {
		auto& channel = m_dm_channels[channel_id];
		auto& person = channel.m_recipients[e["user_id"].get<snowflake>()];
		auto msg_it = ranges::find_if(channel.m_msg_cache.data(), id_equal_to(e["message_id"].get<snowflake>()));
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
			reaction temp;
			temp.m_count = 1;
			temp.m_emoji = std::move(emojiy);
			temp.m_me = e["user_id"].get<snowflake>() == self_user().id();
			m_parent->on_dm_reaction_add(person, channel, std::nullopt, temp, *this);
		}

	}
};

template<>	
void shard::procces_event<event_name::MESSAGE_REACTION_REMOVE>(nlohmann::json& e){
	const auto channel_id = e["channel_id"].get<snowflake>();
	auto it = m_text_channel_map.find(channel_id);
	auto emojiy = e["emoji"].get<partial_emoji>();
	if (it != m_text_channel_map.end()) {
		auto& channel = it->second;
		auto& member = channel.m_guild->m_members[e["user_id"].get<snowflake>()];
		auto msg_it = ranges::find_if(channel.m_msg_cache.data(), id_equal_to(e["message_id"].get<snowflake>()));
		if (msg_it != channel.msg_cache().end()) {
			m_parent->on_guild_reaction_add(
				member, 
				channel,
				*msg_it, 
				remove_reaction(msg_it->m_reactions, emojiy, e["user_id"].get<snowflake>(), self_user().id()), 
				*this
			);
		}else {
			reaction temp;
			temp.m_count = 0;
			temp.m_emoji = std::move(emojiy);
			temp.m_me = false;
			m_parent->on_guild_reaction_add(member, channel, std::nullopt, temp, *this);
		}
	}else {
		auto& channel = m_dm_channels[channel_id];
		auto& person = channel.m_recipients[e["user_id"].get<snowflake>()];
		auto msg_it = ranges::find_if(channel.m_msg_cache.data(), id_equal_to(e["message_id"].get<snowflake>()));

		if (msg_it != channel.msg_cache().end()) {
			m_parent->on_dm_reaction_add(person, channel, *msg_it, remove_reaction(msg_it->m_reactions, emojiy, e["user_id"].get<snowflake>(), self_user().id()), *this);
		}else {
			reaction temp;
			temp.m_count = 0;
			temp.m_emoji = std::move(emojiy);
			temp.m_me = false;
			m_parent->on_dm_reaction_add(person, channel, std::nullopt, temp, *this);
		}
	}

};

template<>
void shard::procces_event<event_name::MESSAGE_REACTION_REMOVE_ALL>(nlohmann::json& e){
	auto it = m_text_channel_map.find(e["channel_id"].get<snowflake>());
	if(it == m_text_channel_map.end()) {
		auto& channel = it->second;
		auto msg_it = ranges::find_if(channel.m_msg_cache.data(), id_equal_to(e["message_id"].get<snowflake>()));
		if(msg_it == channel.msg_cache().end()) {
			m_parent->on_guild_reaction_remove_all(channel, std::nullopt, *this);
		}else {
			msg_it->m_reactions.clear();
			m_parent->on_guild_reaction_remove_all(channel, *msg_it, *this);
		}

	}else {
		auto& channel = m_dm_channels[e["channel_id"].get<snowflake>()];
		auto msg_it = ranges::find_if(channel.m_msg_cache.data(), id_equal_to(e["message_id"].get<snowflake>()));
		if (msg_it == channel.msg_cache().end()) {
			m_parent->on_dm_reaction_remove_all(channel, std::nullopt, *this);
		}else {
			msg_it->m_reactions.clear();
			m_parent->on_dm_reaction_remove_all(channel, *msg_it, *this);
		}
	}

};

template<>
void shard::procces_event<event_name::PRESENCE_UPDATE>(nlohmann::json& e){
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
	if (auto it = e.find("status"); it != e.end())
		user.m_status = string_to_status(it->get<std::string>());

	user.m_roles = e.value("roles", user.m_roles);
	if (auto it = e.find("game"); it != e.end()) {
		auto t = e["game"];
		if (!t.is_null())
			user.m_game.emplace(t.get<activity>());
		else
			user.m_game = std::nullopt;
	}
	m_parent->on_presence_update(user, *this);
};

template<>	
void shard::procces_event<event_name::TYPING_START>(nlohmann::json& e){
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
void shard::procces_event<event_name::USER_UPDATE>(nlohmann::json& e){		
	m_self_user = e.get<user>();
};

template<>	
void shard::procces_event<event_name::VOICE_STATE_UPDATE>(nlohmann::json& e){
	const auto guild_id = e.value("guild_id",snowflake());

	Guild& guild = guild_id.val ? m_guilds[guild_id] : *m_voice_channel_map[e["channel_id"].get<snowflake>()].m_guild;

	const auto user_id = e.value("user_id",self_user().id());

	if(user_id == self_user().id()) {
		m_things_waiting_for_voice_endpoint2[guild.id()].set_value(e["session_id"].get<std::string>());
	}

	const auto it = ranges::find_if(guild.m_voice_states, [&](const auto& a) {return a.user_id() == user_id; });
	if(it != guild.m_voice_states.end()) {
		//update the state
		if(e.count("channel_id")) //wat does this do ;-;
			*it = e.get<voice_state>();
		else 
			guild.m_voice_states.erase(it);		
	}else {
		guild.m_voice_states.push_back(e.get<voice_state>());
	}
};

template<>
void shard::procces_event<event_name::VOICE_SERVER_UPDATE>(nlohmann::json& json){
	const auto guild_id = json["guild_id"].get<snowflake>();
	m_things_waiting_for_voice_endpoint[guild_id].set_value(std::move(json));
};

template<>
void shard::procces_event<event_name::WEBHOOKS_UPDATE>(nlohmann::json&) {
	
}

// ReSharper disable once CppMemberFunctionMayBeConst
void shard::update_presence(const Status s, std::string g) {// NOLINT
	m_status = s;
	m_game_name = std::move(g);
	m_opcode3();
}

rq::send_message shard::send_message(const text_channel& channel, std::string content) {
	nlohmann::json body;
	body["content"] = std::move(content);
	return send_request<rq::send_message>(body.dump(),channel);
}

rq::send_message shard::send_message(const dm_channel& channel, std::string content) {
	nlohmann::json body;
	body["content"] = std::move(content);
	return send_request<rq::send_message>(body.dump(), channel);
}

rq::add_role shard::add_role(const partial_guild& guild, const guild_member& member, const guild_role& role) {
	return send_request<rq::add_role>(guild, member, role);
}

rq::remove_role shard::remove_role(const partial_guild& guild, const guild_member& member,const guild_role& role) {
	return send_request<rq::remove_role>(guild, member, role);
}

rq::modify_member shard::remove_all_roles(const partial_guild& guild, const guild_member& member) {
	return send_request<rq::modify_member>(R"({"roles":{}})"s,guild, member);
}

rq::create_role shard::create_role(const partial_guild& g, std::string name, permission p, int color, bool hoist, bool mentionable) {
	nlohmann::json body;
	body["name"] = std::move(name);
	body["permissions"] = p;
	body["color"] = color;
	body["hoist"] = hoist;
	body["mentionable"] = mentionable;
	return send_request<rq::create_role>(body.dump(), g);
}

rq::delete_role shard::delete_role(const partial_guild& g, const guild_role& role) {
	return send_request<rq::delete_role>(g, role);
}

rq::modify_member shard::change_nick(const guild_member& member, std::string new_nick) {
	nlohmann::json body;
	body["nick"] = std::move(new_nick);
	return send_request<rq::modify_member>(body.dump(), member.guild(), member);
}

rq::modify_member shard::change_nick(const partial_guild& g, const user& member, std::string new_nick) {
	nlohmann::json body;
	body["nick"] = std::move(new_nick);	
	return send_request<rq::modify_member>(body.dump(), g, member);
}

rq::change_my_nick shard::change_my_nick(const partial_guild& g, std::string new_nick) {
	nlohmann::json body;
	body["nick"] = std::move(new_nick);
	return send_request<rq::change_my_nick>(body.dump(), g);
}

rq::kick_member shard::kick_member(const partial_guild& g, const  guild_member& member) {
	return send_request<rq::kick_member>(g, member);
}

rq::ban_member shard::ban_member(const partial_guild& g, const guild_member& member, std::string reason, int days_to_delete_msg) {
	nlohmann::json body;
	body["delete-message-days"] = days_to_delete_msg;
	body["reason"] = std::move(reason);
	return send_request<rq::ban_member>(body.dump(), g, member);
}

rq::unban_member shard::unban_member(const Guild& g, snowflake id) {
	return send_request<rq::unban_member>(g, id);
}

rq::get_messages shard::get_messages(const partial_channel& channel, int n) {
	nlohmann::json body;
	body["limit"] = n;
	return send_request<rq::get_messages>(body.dump(), channel);
}

rq::get_messages shard::get_messages_before(const partial_channel& channel, snowflake id, int n) {
	nlohmann::json body;
	body["limit"] = n;
	body["before"] = id;
	return send_request<rq::get_messages>(body.dump(), channel);
}

rq::get_messages shard::get_messages_after(const partial_channel& channel, snowflake id, int n) {
	nlohmann::json body;
	body["limit"] = n;
	body["after"] = id;
	return send_request<rq::get_messages>(body.dump(), channel);
}

rq::get_messages shard::get_messages_around(const partial_channel& channel, snowflake id, int n) {
	nlohmann::json body;
	body["limit"] = n;
	body["around"] = id;
	return send_request<rq::get_messages>(body.dump(), channel);
}

rq::get_messages shard::get_messages_before(const partial_channel& channel, const partial_message& msg, int n) {
	nlohmann::json body;
	body["limit"] = n;
	body["around"] = msg.id();
	return send_request<rq::get_messages>(body.dump(), channel);
}

rq::get_messages shard::get_messages_after(const partial_channel& channel, const partial_message& msg, int n) {
	nlohmann::json body;
	body["limit"] = n;
	body["around"] = msg.id();
	return send_request<rq::get_messages>(body.dump(), channel);
}

rq::get_messages shard::get_messages_around(const partial_channel& channel, const partial_message& msg, int n) {
	nlohmann::json body;
	body["limit"] = n;
	body["around"] = msg.id();
	return send_request<rq::get_messages>(body.dump(), channel);
}

rq::create_text_channel shard::create_text_channel(const Guild& g, std::string name, std::vector<permission_overwrite> permission_overwrites, bool nsfw) {
	nlohmann::json body;
	body["type"] = 0;
	body["name"] = std::move(name);
	body["permission_overwrites"] = std::move(permission_overwrites);
	body["nsfw"] = nsfw;
	return send_request<rq::create_text_channel>(body.dump(),g);
}

rq::edit_message shard::edit_message(const partial_message& msg, std::string new_content) {
	nlohmann::json body;
	body["content"] = std::move(new_content);
	return send_request<rq::edit_message>(body.dump(), msg);
}

rq::create_voice_channel shard::create_voice_channel(const Guild& g, std::string name, std::vector<permission_overwrite> permission_overwrites, bool nsfw, int bit_rate) {
	nlohmann::json body;
	body["nsfw"] = nsfw;
	body["name"] = std::move(name);
	body["permission_overwrites"] = std::move(permission_overwrites);
	body["bit_rate"] = bit_rate;
	return send_request<rq::create_voice_channel>(body.dump(),g);
}

rq::create_channel_catagory shard::create_channel_catagory(const Guild& g, std::string name,std::vector<permission_overwrite> permission_overwrite, bool nsfw) {
	nlohmann::json body;
	body["nsfw"] = nsfw;
	body["name"] = std::move(name);
	body["permission_overwrites"] = std::move(permission_overwrite);
	return send_request<rq::create_channel_catagory>(body.dump(),g);	
}

rq::delete_message shard::delete_message(const partial_message& msg) {
	return send_request<rq::delete_message>(msg);
}

rq::delete_message_bulk shard::delete_message_bulk(const partial_channel& channel, std::vector<snowflake> msgs){
	nlohmann::json body;
	body["messages"] = std::move(msgs);
	return send_request<rq::delete_message_bulk>(body.dump(),channel);
}

rq::delete_emoji shard::delete_emoji(const partial_guild& g, const emoji& e) {
	return send_request<rq::delete_emoji>(g, e);
}

rq::modify_emoji shard::modify_emoji(Guild & g, emoji & e, std::string s, std::vector<snowflake> v) {
	nlohmann::json body;
	body["name"] = std::move(s);
	body["roles"] = std::move(v);
	return send_request<rq::modify_emoji>(body.dump(), g, e);
}

rq::leave_guild shard::leave_guild(const Guild& g) {
	return send_request<rq::leave_guild>(g);
}

rq::add_reaction shard::add_reaction(const partial_message& msg, const partial_emoji& e) {
	return send_request<rq::add_reaction>(msg, e);
}

rq::typing_start shard::typing_start(const partial_channel& ch) {
	return send_request<rq::typing_start>(ch);
}

rq::delete_channel_permission shard::delete_channel_permissions(const guild_channel& a,const permission_overwrite& b) {
	return send_request<rq::delete_channel_permission>(a, b);
}

rq::list_guild_members shard::list_guild_members(const partial_guild& g, int n, snowflake after) {
	nlohmann::json body;
	body["limit"] = n;
	body["after"] = after;
	return send_request<rq::list_guild_members>(body.dump(),g);
}

rq::edit_channel_permissions shard::edit_channel_permissions(const guild_channel& ch, const permission_overwrite& overwrite) {
	nlohmann::json body = { {"allow",overwrite.allow()},{"deny",overwrite.deny()},{"type",overwrite_type_to_string(overwrite.type())} };
	return send_request<rq::edit_channel_permissions>(body.dump(), ch, overwrite);
}

rq::create_dm shard::create_dm(const user& user) {
	nlohmann::json body;
	body["recipient_id"] = user.id();
	return send_request<rq::create_dm>(body.dump());
}

rq::delete_channel shard::delete_channel(const partial_channel& ch) {
	return send_request<rq::delete_channel>(ch);
}

rq::add_pinned_msg shard::add_pinned_msg(const partial_channel& ch, const partial_message& msg) {
	return send_request<rq::add_pinned_msg>(ch, msg);
}

rq::remove_pinned_msg shard::remove_pinned_msg(const partial_channel& ch, const partial_message& msg) {
	return send_request<rq::remove_pinned_msg>(ch, msg);
}

rq::get_voice_regions shard::get_voice_regions() {
	return send_request<rq::get_voice_regions>();
}

rq::create_channel_invite shard::create_channel_invite(const guild_channel& ch, int max_age, int max_uses, bool temporary,	bool unique) {
	nlohmann::json body;
	body["max_age"] = max_age;
	body["max_uses"] = max_uses;
	body["temporary"] = temporary;
	body["unique"] = unique;
	return send_request<rq::create_channel_invite>(body.dump(), ch);
}

rq::get_invite shard::get_invite(std::string s, int n) {
	if (n != 0) {
		return send_request<rq::get_invite>(std::to_string(n), s);
	}else {
		return send_request<rq::get_invite>("", s);
	}
}

rq::delete_invite shard::delete_invite(std::string s) {
	return send_request<rq::delete_invite>(s);
}

