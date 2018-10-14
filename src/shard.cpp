#include "shard.h"
#include "client.h"
#include "voice_channel.h"
#include "dm_channel.h"
#include <iostream>
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

template<typename T>
T* ptr_or_null(rename_later_4<snowflake,T>& in,snowflake key) {
	if (key.val)
		return &in[key];
	return nullptr;
}

void shard::doStuff(nlohmann::json stuffs) {
	std::cout << stuffs.dump(1) << std::endl;
	const auto op = stuffs["op"].get<int>();
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

void shard::set_up_request(boost::beast::http::request<boost::beast::http::string_body>& req) const{
	m_parent->set_up_request(req);
}

void shard::m_opcode0(nlohmann::json data, eventName event, size_t s) {
	m_seqNum = std::max(s, m_seqNum);
	switch (event) {
	case eventName::HELLO: m_procces_event<eventName::HELLO>(data); break;
	case eventName::READY: m_procces_event<eventName::READY>(data); break;;
	case eventName::RESUMED: m_procces_event<eventName::RESUMED>(data); break;;
	case eventName::INVALID_SESSION: m_procces_event<eventName::INVALID_SESSION>(data); break;;
	case eventName::CHANNEL_CREATE: m_procces_event<eventName::CHANNEL_CREATE>(data); break;;
	case eventName::CHANNEL_UPDATE: m_procces_event<eventName::CHANNEL_UPDATE>(data); break;;
	case eventName::CHANNEL_DELETE: m_procces_event<eventName::CHANNEL_DELETE>(data); break;;
	case eventName::CHANNEL_PINS_UPDATE: m_procces_event<eventName::CHANNEL_PINS_UPDATE>(data); break;;
	case eventName::GUILD_CREATE: m_procces_event<eventName::GUILD_CREATE>(data); break;;
	case eventName::GUILD_UPDATE: m_procces_event<eventName::GUILD_UPDATE>(data); break;;
	case eventName::GUILD_DELETE: m_procces_event<eventName::GUILD_DELETE>(data); break;;
	case eventName::GUILD_BAN_ADD: m_procces_event<eventName::GUILD_BAN_ADD>(data); break;;
	case eventName::GUILD_BAN_REMOVE: m_procces_event<eventName::GUILD_BAN_REMOVE>(data); break;;
	case eventName::GUILD_EMOJI_UPDATE: m_procces_event<eventName::GUILD_EMOJI_UPDATE>(data); break;;
	case eventName::GUILD_INTEGRATIONS_UPDATE: m_procces_event<eventName::GUILD_INTEGRATIONS_UPDATE>(data); break;;
	case eventName::GUILD_MEMBER_ADD: m_procces_event<eventName::GUILD_MEMBER_ADD>(data); break;;
	case eventName::GUILD_MEMBER_REMOVE: m_procces_event<eventName::GUILD_MEMBER_REMOVE>(data); break;;
	case eventName::GUILD_MEMBER_UPDATE: m_procces_event<eventName::GUILD_MEMBER_UPDATE>(data); break;;
	case eventName::GUILD_MEMBER_CHUNK: m_procces_event<eventName::GUILD_MEMBER_CHUNK>(data); break;;
	case eventName::GUILD_ROLE_CREATE: m_procces_event<eventName::GUILD_ROLE_CREATE>(data); break;;
	case eventName::GUILD_ROLE_UPDATE: m_procces_event<eventName::GUILD_ROLE_UPDATE>(data); break;;
	case eventName::GUILD_ROLE_DELETE: m_procces_event<eventName::GUILD_ROLE_DELETE>(data); break;;
	case eventName::MESSAGE_CREATE: m_procces_event<eventName::MESSAGE_CREATE>(data); break;;
	case eventName::MESSAGE_UPDATE: m_procces_event<eventName::MESSAGE_UPDATE>(data); break;;
	case eventName::MESSAGE_DELETE: m_procces_event<eventName::MESSAGE_DELETE>(data); break;;
	case eventName::MESSAGE_DELETE_BULK: m_procces_event<eventName::MESSAGE_DELETE_BULK>(data); break;;
	case eventName::MESSAGE_REACTION_ADD: m_procces_event<eventName::MESSAGE_REACTION_ADD>(data); break;;
	case eventName::MESSAGE_REACTION_REMOVE: m_procces_event<eventName::MESSAGE_REACTION_REMOVE>(data); break;;
	case eventName::MESSAGE_REACTION_REMOVE_ALL: m_procces_event<eventName::MESSAGE_REACTION_REMOVE_ALL>(data); break;;
	case eventName::PRESENCE_UPDATE: m_procces_event<eventName::PRESENCE_UPDATE>(data); break;;
	case eventName::TYPING_START: m_procces_event<eventName::TYPING_START>(data); break;;
	case eventName::USER_UPDATE: m_procces_event<eventName::USER_UPDATE>(data); break;;
	case eventName::VOICE_STATE_UPDATE: m_procces_event<eventName::VOICE_STATE_UPDATE>(data); break;;
	case eventName::VOICE_SERVER_UPDATE: m_procces_event<eventName::VOICE_SERVER_UPDATE>(data); break;;
	case eventName::WEBHOOKS_UPDATE:  m_procces_event<eventName::WEBHOOKS_UPDATE>(data); break;;
	default:;
	}
}

void shard::m_opcode1() const {
	if (is_disconnected())return;
	m_client->send(
R"({
	"op": 1,
	"d" : null
})");
}

void shard::m_opcode2() const {
	m_sendIdentity();
}

void shard::m_opcode3() const {
	nlohmann::json val;
	val["op"] = 3;
	val["data"] = m_parent->presence();
	m_client->send(val.dump().c_str());
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
	const std::string t = s.dump();
	m_client->send(t.c_str());
}

void shard::m_opcode9(const nlohmann::json& d) const {
	std::this_thread::sleep_for(5s);
	if (d.get<bool>())
		m_opcode6();
	else
		m_opcode2();
}

void shard::send_heartbeat() {
	if (!m_op11.load()) {
		reconnect();
	}
	m_opcode1();
	m_op11 = false;
	m_parent->heartbeat_sender.execute(std::make_pair([this]() {send_heartbeat(); }, std::chrono::steady_clock::now() + std::chrono::milliseconds(m_hb_interval)));
}

void shard::m_opcode10(nlohmann::json& stuff) {
	m_hb_interval = stuff["heartbeat_interval"].get<int>();
	send_heartbeat();
	m_sendIdentity();
}

void shard::m_opcode11(nlohmann::json& data) {
	m_op11 = true;
}

void shard::reconnect() {
	if (!is_disconnected())
		close_connection(4000);
	m_parent->reconnect(this, m_shard_number);
}

void shard::send_resume() const {
	const std::string temp(R"({
		"op":6,
		"d":{
			"token":)"s + m_parent->token() + R"(
			"session_id:)"s + session_id + R"(
			"seq:")" + std::to_string(m_seqNum) + R"(
		}
	}
	)"s);
	m_client->send(temp.c_str());
}


void shard::m_sendIdentity()const {
	const std::string identity = R"({
		"op":2,
		"d":{
			"token":")" + m_parent->token() + R"(",
			"properties":{
				"$os":")"s + s_os + R"(",
				"$browser":"cerwy",
				"$device":"cerwy"
			},
			"compress":false,
			"large_threshold":250,
			"shard":[)"s + std::to_string(m_shard_number) + "," + std::to_string(m_parent->num_shards()) + R"(],	
			"presence":)"s + m_parent->presence().dump() +
		"}"s + "}";
	m_client->send(identity.c_str());
}



template <>
void shard::m_procces_event<eventName::READY>(const nlohmann::json& event) {
	m_trace2 = event["_trace"];
	m_parent->self_user() = event["user"].get<user>();
	session_id = event["session_id"].get<std::string>();
	//ignore unavailable guilds;
	m_guilds.reserve(event["guilds"].size());

	const auto& private_channels = event["private_channels"].get_ref<const nlohmann::json::array_t&>();
	m_dm_channels.reserve(private_channels.size());
	for (const auto& c : private_channels)
		insert_proj_as_key(m_dm_channels, c.get<dm_channel>(),get_id);	
}

template <>
void shard::m_procces_event<eventName::GUILD_CREATE>(const nlohmann::json& data) {
	Guild& guild = m_guilds.insert(std::make_pair(data["id"].get<snowflake>(),data.get<Guild>())).first->second;
	for (auto& member : guild.m_members) 
		member.m_guild = &guild;
	
	const auto channels = data["channels"].get_ref<const nlohmann::json::array_t&>();
	guild.m_text_channels.reserve(channels.size());
	guild.m_voice_channels.reserve(std::max(3ull,channels.size() / 10));//random number
	for (const auto& item : channels) {
		if(const auto type = item["type"].get<int>(); type==0) {//text
			auto& channel = m_text_channels.insert(std::make_pair(item["id"].get<snowflake>(), item.get<text_channel>())).first->second;
			//channel.m_shard = this;
			channel.m_guild_id = guild.id();
			guild.m_text_channels.push_back(channel.id());
			channel.m_guild = &guild;
		}else if(type==1) {//DM
		}else if(type==2) {//voice
			auto& channel = m_voice_channels.insert(std::make_pair(item["id"].get<snowflake>(), item.get<voice_channel>())).first->second;
			//channel.m_shard = this;
			channel.m_guild_id = guild.id();
			guild.m_voice_channels.push_back(channel.id());
			channel.m_guild = &guild;
		}else if(type==3) {//group dm
		}else if(type==4) {//guild catagory
			auto& channel = m_channel_catagories.insert(std::make_pair(item["id"].get<snowflake>(), item.get<channel_catagory>())).first->second;
			//channel.m_shard = this;
			channel.m_guild_id = guild.id();
			guild.m_channel_catagories.push_back(channel.id());
			channel.m_guild = &guild;
		}
	}
	for(const auto& channel_id:guild.m_text_channels /* | ranges::transform([&](auto id)->auto&{return m_text_channels[ch];}) | ranges::filter([](auto&& ch){return ch.has_parent();})*/) {
		auto& channel = m_text_channels[channel_id];
		if(channel.m_parent_id.val) 
			channel.m_parent = &m_channel_catagories[channel.m_parent_id];		
	}
	for (const auto& channel_id : guild.m_voice_channels) {
		auto& channel = m_voice_channels[channel_id];
		if (channel.m_parent_id.val) 
			channel.m_parent = &m_channel_catagories[channel.m_parent_id];		
	}
	const auto members = to_map(guild.m_members);

	for(const auto& presence: data["presences"]) {		
		auto& member = *members.at(presence["user"]["id"].get<snowflake>());
		member.m_status = string_to_status(presence["status"].get_ref<const nlohmann::json::string_t&>());
		const auto temp = presence["game"];
		if(!temp.is_null())
			member.m_game.emplace(temp.get<activity>());
	}
	guild.m_shard = this;
}

template<typename T>
struct always_map{
	constexpr always_map(T a):m_thing(std::move(a)){}

	template<typename A>
	T& operator[](A&&) noexcept{
		return m_thing;
	}
	template<typename A>
	const T& operator[](A&&) const noexcept{
		return m_thing;
	}

	// ReSharper disable CppMemberFunctionMayBeStatic
	size_t size()const noexcept {
		// ReSharper restore CppMemberFunctionMayBeStatic
		return 1;
	}

private:
	T m_thing;
};

template <>
void shard::m_procces_event<eventName::MESSAGE_CREATE>(const nlohmann::json& e) {
	const auto channel_id = e["channel_id"].get<snowflake>();
	if (auto it = m_text_channels.find(channel_id);it != m_text_channels.end()) {//dm msg
		text_channel& ch = it->second;

		auto msg = [&]{
			if(e["mentions"].size())
				return createMsg<guild_text_message>(ch, e, to_map(m_guilds[ch.m_guild_id].m_members));
			const auto ita = std::lower_bound(m_guilds[ch.m_guild_id].m_members.begin(), m_guilds[ch.m_guild_id].m_members.end(),  e["author"]["id"].get<snowflake>(),id_comp);
			return createMsg<guild_text_message>(ch, e, always_map<guild_member*>(&*ita));
		}();

		m_parent->on_guild_text_msg(ch.m_add_msg(std::move(msg)), *this);
	}else {//guild text msg		
		dm_channel& ch = m_dm_channels[channel_id];
		auto msg = createMsg<dm_message>(ch, e, to_map(ch.m_recipients));
		m_parent->on_dm_msg(ch.m_add_msg(std::move(msg)), *this);
	}
}

template <>
void shard::m_procces_event<eventName::CHANNEL_CREATE>(const nlohmann::json& channel_json) {	
	const auto id = channel_json["id"].get<snowflake>();
	const auto type = channel_json["type"].get<int>();
	if (type == 0) {//text
		auto& channel = m_text_channels.insert(std::make_pair(id, channel_json.get<text_channel>())).first->second;
		m_guilds[channel.m_guild_id].m_text_channels.push_back(id);
		channel.m_guild = &m_guilds[channel.m_guild_id];
		channel.m_parent = ptr_or_null(m_channel_catagories, channel.m_parent_id);
		m_parent->on_guild_text_channel_create(channel,*this);
	}else if (type == 1) {//DM
		auto& channel = m_dm_channels.insert(std::make_pair(id, channel_json.get<dm_channel>())).first->second;
		m_parent->on_dm_channel_create(channel,*this);
	}else if (type == 2) {//voice
		auto& channel = m_voice_channels.insert(std::make_pair(id, channel_json.get<voice_channel>())).first->second;
		m_guilds[channel.m_guild_id].m_voice_channels.push_back(id);
		channel.m_guild = &m_guilds[channel.m_guild_id];
		channel.m_parent = ptr_or_null(m_channel_catagories, channel.m_parent_id);
		m_parent->on_guild_voice_channel_create(channel,*this);
	}else if (type == 3) {//group dm
		auto& channel = m_dm_channels.insert(std::make_pair(id, channel_json.get<dm_channel>())).first->second;
		m_parent->on_dm_channel_create(channel, *this);
	}else if (type == 4) {//guild catagory
		auto& channel = m_channel_catagories.insert(std::make_pair(id, channel_json.get<channel_catagory>())).first->second;
		m_guilds[channel.m_guild_id].m_channel_catagories.push_back(id);
		channel.m_guild = &m_guilds[channel.m_guild_id];
		m_parent->on_guild_channel_catagory_create(channel,*this);
	}	
}

template <>
void shard::m_procces_event<eventName::CHANNEL_DELETE>(const nlohmann::json& e) {
	const auto channel_id = e["id"].get<snowflake>();
	if (auto it = m_text_channels.find(channel_id); it != m_text_channels.end()) {
		text_channel channel = std::move(it->second);
		m_text_channels.erase(it);
		Guild& g = m_guilds[channel.guild_id()];
		erase_first_quick(g.m_text_channels, channel_id);
		m_parent->on_text_channel_delete(channel, *this);
	}else {
		auto it2 = m_dm_channels.find(channel_id);
		dm_channel d = std::move(it2->second);
		m_dm_channels.erase(it2);
		m_parent->on_dm_channel_delete(d, *this);
	}
}

template <>
void shard::m_procces_event<eventName::GUILD_MEMBER_ADD>(const nlohmann::json& e) {	
	auto member = e.get<guild_member>();
	Guild& guild = m_guilds[member.guild().id()];
	//assert(std::is_sorted(guild.m_members.begin(), guild.m_members.end(), [](const auto& a, const auto& b) {return a.id().val < b.id().val; }));
	const auto it = guild.m_members.insert(
		std::lower_bound(guild.m_members.begin(), guild.m_members.end(), member.id(), id_comp),
		std::move(member)
	);
	it->m_guild = &guild;
	m_parent->on_guild_member_add(*it,*this);
}

template <>
void shard::m_procces_event<eventName::GUILD_MEMBER_REMOVE>(const nlohmann::json& e) {
	Guild& guild = m_guilds[e["guild_id"].get<snowflake>()];
	const auto user_id = e["user"]["id"].get<snowflake>();
	assert(std::is_sorted(guild.m_members.begin(), guild.m_members.end(), [](const auto& a, const auto& b) {return a.id().val < b.id().val; }));
	auto it = std::lower_bound(guild.m_members.begin(), guild.m_members.end(), user_id, id_comp);
	--guild.m_member_count;
	if(it == guild.m_members.end() || it->id() != user_id) {//;-;
		guild_member member;
		from_json(e["user"], static_cast<user&>(member));
		member.m_guild = &guild;
		m_parent->on_guild_member_remove(member, false,*this);
	}else {		
		auto member = std::move(*it);
		guild.m_members.erase(it);
		m_parent->on_guild_member_remove(member,true,*this);
	}
}

template<> void shard::m_procces_event<eventName::RESUMED>(const nlohmann::json& e){
	m_trace = e["_trace"].get<std::vector<std::string>>();
};

template<>	void shard::m_procces_event<eventName::HELLO>(const nlohmann::json& json){
	m_trace = json.at("_trace").get<std::vector<std::string>>();
	m_hb_interval = json["heartbeat_interval"].get<int>();
};

template<>	void shard::m_procces_event<eventName::INVALID_SESSION>(const nlohmann::json&){};


template<>	void shard::m_procces_event<eventName::CHANNEL_UPDATE>(const nlohmann::json& e){
	const auto channel_id = e["id"].get<snowflake>();
	const auto type = e["type"].get<int>();
	if(type == 0) {
		auto& channel = m_text_channels[channel_id];
		channel = e.get<text_channel>();
		channel.m_parent = ptr_or_null(m_channel_catagories, channel.m_parent_id);
	}else if(type == 1) {
		m_dm_channels[channel_id] = e.get<dm_channel>();
	}else if(type == 2) {
		auto& channel = m_voice_channels[channel_id];
		channel = e.get<voice_channel>();
		channel.m_parent = ptr_or_null(m_channel_catagories, channel.m_parent_id);
	}else if(type == 3) {//;-;
		
	}else if(type == 4) {
		m_channel_catagories[channel_id] = e.get<channel_catagory>();
	}
}

template<>	void shard::m_procces_event<eventName::CHANNEL_PINS_UPDATE>(const nlohmann::json&){

};

template<>	void shard::m_procces_event<eventName::GUILD_UPDATE>(const nlohmann::json& e){
	const auto guild_id = e["id"].get<snowflake>();
	Guild& g = m_guilds[guild_id];
	from_json(e, static_cast<partial_guild&>(g));
	m_parent->on_guild_update(g, *this);
};

template<>	void shard::m_procces_event<eventName::GUILD_DELETE>(const nlohmann::json& e){
	const auto unavailable = e["unavailable"].get<bool>();
	const Guild g = std::move(m_guilds.extract(e["id"].get<snowflake>()).mapped());
	m_parent->on_guild_remove(g, unavailable, *this);
};


template<>	void shard::m_procces_event<eventName::GUILD_BAN_ADD>(const nlohmann::json& e){
	Guild& g = m_guilds[e["id"].get<snowflake>()];
	auto member = e.get<user>();
	m_parent->on_ban_add(g, member, *this);
};


template<>	void shard::m_procces_event<eventName::GUILD_BAN_REMOVE>(const nlohmann::json& e){
	auto member = e.get<user>();
	Guild& g = m_guilds[member.id()];
	m_parent->on_ban_remove(g, member, *this);
};


template<>	void shard::m_procces_event<eventName::GUILD_EMOJI_UPDATE>(const nlohmann::json& e){
	m_guilds[e["id"].get<snowflake>()].m_emojis = e["emojis"].get<std::vector<emoji>>();
};


template<>	void shard::m_procces_event<eventName::GUILD_INTEGRATIONS_UPDATE>(const nlohmann::json&){
	//what is this ;-;
};

template<>	void shard::m_procces_event<eventName::GUILD_MEMBER_UPDATE>(const nlohmann::json& e){
	auto& guild = m_guilds[e["guild_id"].get<snowflake>()];
	const auto user_ = e["user"];
	const auto user_id = user_["id"].get<snowflake>();
	const auto it = std::lower_bound(guild.m_members.begin(), guild.m_members.end(), user_id, id_comp);
	
	if(it==guild.m_members.end() || it->id() != user_id) {//;-; create guild_member and add it to m_members
		guild_member member;
		from_json(user_, static_cast<user&>(member));
		member.m_roles = e["roles"].get<std::vector<snowflake>>();		
		if(const auto it2 = e.find("nick"); it2 != e.end())
			member.m_nick = e["nick"].is_null()?"":e["nick"].get<std::string>();
		member.m_guild = &guild;
		m_parent->on_guild_member_update(*guild.m_members.insert(it,std::move(member)),*this); ;
	}else{
		from_json(user_, static_cast<user&>(*it));
		it->m_roles = e["roles"].get<std::vector<snowflake>>();		
		if(const auto it2 = e.find("nick"); it2 != e.end())
			it->m_nick = e["nick"].is_null()? it->m_nick : e["nick"].get<std::string>();
		m_parent->on_guild_member_update(*it, *this);
	}
	
};
//FIX THIS SOMEHOW ;-;, it's bad D:
template<>	void shard::m_procces_event<eventName::GUILD_MEMBER_CHUNK>(const nlohmann::json& e){//really bad ;-;
	Guild& g = m_guilds[e["guild_id"].get<snowflake>()];
	for(const auto& i:e["members"]) {
		g.m_members.push_back(i.get<guild_member>());
	}
	if(g.m_members.size() == g.m_member_count)// it's done sending stuff 
		std::sort(g.m_members.begin(), g.m_members.end(), [](const auto& a,const auto& b) {return a.id() < b.id(); });
};


template<>	void shard::m_procces_event<eventName::GUILD_ROLE_CREATE>(const nlohmann::json& e){
	Guild& guild = m_guilds[e["guild_id"].get<snowflake>()];
	m_parent->on_role_create(guild, guild.m_roles.insert(std::make_pair(e["role"]["id"].get<snowflake>(),e["role"].get<guild_role>())).first->second, *this);
};

template<>	void shard::m_procces_event<eventName::GUILD_ROLE_UPDATE>(const nlohmann::json& e){
	try{
		Guild& guild = m_guilds[e["guild_id"].get<snowflake>()];
		auto new_role = e["role"].get<guild_role>();
		guild_role& old_role = guild.m_roles[new_role.id()];
		std::swap(old_role, new_role);
		//old_role is now new_role
		m_parent->on_role_update(guild,new_role,old_role , *this);
	}catch(...) {
		//;-;	
	}
};

template<>	void shard::m_procces_event<eventName::GUILD_ROLE_DELETE>(const nlohmann::json& e){
	Guild& guild = m_guilds[e["guild_id"].get<snowflake>()];
	const auto role_id = e["role_id"].get<snowflake>();	
	const guild_role old_role = std::move(guild.m_roles.extract(role_id).mapped());
	for(auto& member:guild.m_members) 
		erase_first_quick(member.m_roles, old_role.id());	
	m_parent->on_role_delete(guild, old_role, *this);
};

template<>	void shard::m_procces_event<eventName::MESSAGE_UPDATE>(const nlohmann::json& e){
	try{
	const auto channel_id = e["channel_id"].get<snowflake>();
	if (auto it = m_text_channels.find(channel_id); it != m_text_channels.end()) {//guild text msg	
		text_channel& ch = it->second;		
		auto msg = createMsgUpdate<guild_msg_update>(ch, e, to_map(m_guilds[ch.m_guild_id].m_members));
		auto it2 = std::find_if(ch.m_msg_cache.data().begin(), ch.m_msg_cache.data().end(), id_equal_to(msg.id()));
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
		auto msg = createMsgUpdate<dm_msg_update>(ch, e, to_map(ch.m_recipients));
		auto it2 = std::find_if(ch.m_msg_cache.data().begin(), ch.m_msg_cache.data().end(), id_equal_to(msg.id()));
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
	catch(...){}
};

template<>	void shard::m_procces_event<eventName::MESSAGE_DELETE>(const nlohmann::json& e){
	const auto channel_id = e["channel_id"].get<snowflake>();
	const auto msg_id = e["id"].get<snowflake>();
	if (auto it = m_text_channels.find(channel_id); it != m_text_channels.end()) {//guild text msg
		text_channel& ch = it->second;
		auto it2 = std::find_if(ch.msg_cache().begin(), ch.msg_cache().end(), id_equal_to(msg_id));
		if (it2 != ch.msg_cache().end()) {
			m_parent->on_guild_msg_delete(std::move(*it2), channel_id, *this);
		}else {
			m_parent->on_guild_msg_delete(std::nullopt, channel_id, *this);
		}
	}else {//dm msg
		dm_channel& ch = m_dm_channels[channel_id];
		auto it2 = std::find_if(ch.msg_cache().begin(), ch.msg_cache().end(), id_equal_to(msg_id));
		if (it2 != ch.msg_cache().end()) {
			m_parent->on_dm_msg_delete(std::move(*it2), channel_id, *this);
		}else {
			m_parent->on_dm_msg_delete(std::nullopt, channel_id, *this);
		}
	}
};

template<>	void shard::m_procces_event<eventName::MESSAGE_DELETE_BULK>(const nlohmann::json& e){
	try{
		m_parent->on_message_bulk(e["ids"].get<std::vector<snowflake>>(),m_text_channels.at(e["channel_id"].get<snowflake>()),*this);
	}catch (...) {//;-;		
		m_parent->on_dm_message_bulk(e["ids"].get<std::vector<snowflake>>(), m_dm_channels[e["channel_id"].get<snowflake>()], *this);
	}
};

template<>	void shard::m_procces_event<eventName::MESSAGE_REACTION_ADD>(const nlohmann::json& e){
	const auto channel_id = e["channel_id"].get<snowflake>();
	auto it = m_text_channels.find(channel_id);
	auto emojiy = e["emoji"].get<partial_emoji>();

	if (it != m_text_channels.end()) {
		auto& channel = it->second;
		//assert(std::is_sorted(channel.m_guild->m_members.begin(), channel.m_guild->m_members.end(), [](const auto& a, const auto& b) {return a.id().val < b.id().val; }));
		auto& member = *std::lower_bound(channel.m_guild->m_members.begin(),channel.m_guild->m_members.end(),e["user_id"].get<snowflake>(),id_comp);
		auto msg_it = std::find_if(it->second.m_msg_cache.data().begin(), it->second.m_msg_cache.data().end(), id_equal_to(e["message_id"].get<snowflake>()));
		if (msg_it != it->second.msg_cache().end()) {
			msg_it->m_reactions;		
			m_parent->on_guild_reaction_add(member, channel, *msg_it, update_reactions<1>(msg_it->m_reactions, emojiy, e["user_id"].get<snowflake>(), m_parent->self_user().id()), *this);
		}else {
			reaction temp;
			temp.m_count = 1;
			temp.m_emoji = std::move(emojiy);
			temp.m_me = e["user_id"].get<snowflake>() == m_parent->self_user().id();
			m_parent->on_guild_reaction_add(member, channel, std::nullopt, temp, *this);
		}
	}else {
		auto& channel = m_dm_channels[channel_id];
		auto& person = *std::find_if(channel.m_recipients.begin(), channel.m_recipients.end(), id_equal_to(e["user_id"].get<snowflake>()));
		auto msg_it = std::find_if(channel.m_msg_cache.data().begin(), channel.m_msg_cache.data().end(), id_equal_to(e["message_id"].get<snowflake>()));
		if(msg_it != channel.msg_cache().end()) {
			m_parent->on_dm_reaction_add(person, channel, *msg_it, update_reactions<1>(msg_it->m_reactions, emojiy, e["user_id"].get<snowflake>(), m_parent->self_user().id()), *this);
		}else {
			reaction temp;
			temp.m_count = 1;
			temp.m_emoji = std::move(emojiy);
			temp.m_me = e["user_id"].get<snowflake>() == m_parent->self_user().id();
			m_parent->on_dm_reaction_add(person, channel, std::nullopt, temp, *this);
		}
	}
};

template<>	void shard::m_procces_event<eventName::MESSAGE_REACTION_REMOVE>(const nlohmann::json& e){
	const auto channel_id = e["channel_id"].get<snowflake>();
	auto it = m_text_channels.find(channel_id);
	auto emojiy = e["emoji"].get<partial_emoji>();
	if (it != m_text_channels.end()) {
		auto& channel = it->second;
		assert(std::is_sorted(channel.m_guild->m_members.begin(), channel.m_guild->m_members.end(), [](const auto& a, const auto& b) {return a.id().val < b.id().val; }));
		auto& member = *std::lower_bound(channel.m_guild->m_members.begin(), channel.m_guild->m_members.end(), e["user_id"].get<snowflake>(), id_comp);
		auto msg_it = std::find_if(it->second.m_msg_cache.data().begin(), it->second.m_msg_cache.data().end(), id_equal_to(e["message_id"].get<snowflake>()));
		if (msg_it != it->second.msg_cache().end()) {
			msg_it->m_reactions;
			m_parent->on_guild_reaction_add(member, channel, *msg_it, update_reactions<-1>(msg_it->m_reactions, emojiy, e["user_id"].get<snowflake>(), m_parent->self_user().id()), *this);
		}
		else {
			reaction temp;
			temp.m_count = 0;
			temp.m_emoji = std::move(emojiy);
			temp.m_me = false;
			m_parent->on_guild_reaction_add(member, channel, std::nullopt, temp, *this);
		}
	}
	else {
		auto& channel = m_dm_channels[channel_id];
		auto& person = *std::find_if(channel.m_recipients.begin(), channel.m_recipients.end(), id_equal_to(e["user_id"].get<snowflake>()));
		auto msg_it = std::find_if(channel.m_msg_cache.data().begin(), channel.m_msg_cache.data().end(), id_equal_to(e["message_id"].get<snowflake>()));
		if (msg_it != channel.msg_cache().end()) {
			m_parent->on_dm_reaction_add(person, channel, *msg_it, update_reactions<-1>(msg_it->m_reactions, emojiy, e["user_id"].get<snowflake>(), m_parent->self_user().id()), *this);
		}
		else {
			reaction temp;
			temp.m_count = 0;
			temp.m_emoji = std::move(emojiy);
			temp.m_me = false;
			m_parent->on_dm_reaction_add(person, channel, std::nullopt, temp, *this);
		}
	}

};

template<>	void shard::m_procces_event<eventName::MESSAGE_REACTION_REMOVE_ALL>(const nlohmann::json& e){
	auto it = m_text_channels.find(e["channel_id"].get<snowflake>());
	if(it == m_text_channels.end()) {
		auto& channel = it->second;
		auto msg_it = std::find_if(channel.m_msg_cache.data().begin(), channel.m_msg_cache.data().end(), id_equal_to(e["message_id"].get<snowflake>()));
		if(msg_it == channel.msg_cache().end()) {
			m_parent->on_guild_reaction_remove_all(channel, std::nullopt, *this);
		}else {
			msg_it->m_reactions.clear();
			m_parent->on_guild_reaction_remove_all(channel, *msg_it, *this);
		}

	}else {
		auto& channel = m_dm_channels[e["channel_id"].get<snowflake>()];
		auto msg_it = std::find_if(channel.m_msg_cache.data().begin(), channel.m_msg_cache.data().end(), id_equal_to(e["message_id"].get<snowflake>()));
		if (msg_it == channel.msg_cache().end()) {
			m_parent->on_dm_reaction_remove_all(channel, std::nullopt, *this);
		}else {
			msg_it->m_reactions.clear();
			m_parent->on_dm_reaction_remove_all(channel, *msg_it, *this);
		}
	}

};

template<>	void shard::m_procces_event<eventName::PRESENCE_UPDATE>(const nlohmann::json& e){
	auto& guild = m_guilds[e["guild_id"].get<snowflake>()];
	auto& user = *std::lower_bound(guild.m_members.begin(), guild.m_members.end(), e["user"]["id"].get<snowflake>(), id_comp);
	if (auto it = e.find("status"); it != e.end())
		user.m_status = string_to_status(e["status"].get<std::string>());
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

template<>	void shard::m_procces_event<eventName::TYPING_START>(const nlohmann::json& e){
	const auto channel_id = e["channel_id"].get<snowflake>();
	const auto user_id = e["user_id"].get<snowflake>();
	const auto it = m_text_channels.find(channel_id);	
	if (it != m_text_channels.end()) {
		Guild& guild = *(it->second.m_guild);
		assert(std::is_sorted(guild.m_members.begin(), guild.m_members.end(), [](const auto& a, const auto& b) {return a.id().val < b.id().val; }));
		auto& member = *std::lower_bound(guild.m_members.begin(), guild.m_members.end(), user_id, id_comp);
		m_parent->on_guild_typing_start(member, it->second, *this);
	}else {
		dm_channel& channel = m_dm_channels[channel_id];
		auto& member = *std::find_if(channel.m_recipients.begin(), channel.m_recipients.end(), id_equal_to(user_id));
		m_parent->on_dm_typing_start(member, channel, *this);
	}
};

template<>	void shard::m_procces_event<eventName::USER_UPDATE>(const nlohmann::json& e){	

};

template<>	void shard::m_procces_event<eventName::VOICE_STATE_UPDATE>(const nlohmann::json& e){
	Guild& guild = m_guilds[e["guild_id"].get<snowflake>()];
	const auto user_id = e["user_id"].get<snowflake>();
	auto it = std::find_if(guild.m_voice_states.begin(), guild.m_voice_states.end(), [&](const auto& a) {return a.user_id() == user_id; });
	if(it != guild.m_voice_states.end()) {
		if(e["channel_id"].get<snowflake>().val == 0) 
			*it = e.get<voice_state>();
		else 
			guild.m_voice_states.erase(it);		
	}else {
		guild.m_voice_states.push_back(e.get<voice_state>());
	}
};

template<>	void shard::m_procces_event<eventName::VOICE_SERVER_UPDATE>(const nlohmann::json&){

};

template<>	void shard::m_procces_event<eventName::WEBHOOKS_UPDATE>(const nlohmann::json&){};

// ReSharper disable once CppMemberFunctionMayBeConst
void shard::update_presence(const Status s, std::string g) {
	m_parent->status = s;
	m_parent->gameName = std::move(g);
	m_opcode3();
}


rq::send_message shard::send_message(const text_channel& channel, std::string content) {
	nlohmann::json body;
	body["content"] = std::move(content);
	return m_send_things<rq::send_message>(body.dump(),channel);
}

rq::send_message shard::send_message(const dm_channel& channel, std::string content) {
	nlohmann::json body;
	body["content"] = std::move(content);
	return m_send_things<rq::send_message>(body.dump(), channel);
}

rq::add_role shard::add_role(const partial_guild& guild, const guild_member& member, const guild_role& role) {
	return m_send_things<rq::add_role>(guild, member, role);
}

rq::remove_role shard::remove_role(const partial_guild& guild, const  guild_member& member,const guild_role& role) {
	return m_send_things<rq::remove_role>(guild, member, role);
}

rq::modify_member shard::remove_all_roles(const partial_guild& guild, const  guild_member& member) {
	return m_send_things<rq::modify_member>(R"({"roles":{}})"s,guild, member);
}

rq::create_role shard::create_role(const partial_guild& g, std::string_view name, permission p, int color, bool hoist, bool mentionable) {
	nlohmann::json body;
	body["name"] = std::string(name);
	body["permissions"] = p;
	body["color"] = color;
	body["hoist"] = hoist;
	body["mentionable"] = mentionable;
	return m_send_things<rq::create_role>(body.dump(), g);
}

rq::delete_role shard::delete_role(const partial_guild& g, const guild_role& role) {
	return m_send_things<rq::delete_role>(g, role);
}

rq::modify_member shard::change_nick(const guild_member& member, std::string new_nick) {
	nlohmann::json body;
	body["nick"] = std::move(new_nick);
	return m_send_things<rq::modify_member>(body.dump(), member.guild(), member);
}

rq::modify_member shard::change_nick(const partial_guild& g, const user& member, std::string new_nick) {
	nlohmann::json body;
	body["nick"] = std::move(new_nick);
	return m_send_things<rq::modify_member>(body.dump(), g, member);
}

rq::change_my_nick shard::change_my_nick(const partial_guild& g, std::string_view new_nick) {
	nlohmann::json body;
	body["nick"] = std::string(new_nick);
	return m_send_things<rq::change_my_nick>(body.dump(), g);
}

rq::kick_member shard::kick_member(const partial_guild& g, const  guild_member& member) {
	return m_send_things<rq::kick_member>(g, member);
}

rq::ban_member shard::ban_member(const partial_guild& g, const  guild_member& member, std::string reason, int days_to_delete_msg) {
	nlohmann::json body;
	body["delete-message-days"] = days_to_delete_msg;
	body["reason"] = std::move(reason);
	return m_send_things<rq::ban_member>(body.dump(), g, member);
}

rq::unban_member shard::unban_member(const Guild& g, snowflake id) {
	return m_send_things<rq::unban_member>(g, id);
}

rq::get_messages shard::get_messages(const partial_channel& channel, int n) {
	nlohmann::json body;
	body["limit"] = n;
	return m_send_things<rq::get_messages>(body.dump(), channel);
}

rq::get_messages shard::get_messages_before(const partial_channel& channel, snowflake id, int n) {
	nlohmann::json body;
	body["limit"] = n;
	body["before"] = id;
	return m_send_things<rq::get_messages>(body.dump(), channel);
}

rq::get_messages shard::get_messages_after(const partial_channel& channel, snowflake id, int n) {
	nlohmann::json body;
	body["limit"] = n;
	body["after"] = id;
	return m_send_things<rq::get_messages>(body.dump(), channel);
}

rq::get_messages shard::get_messages_around(const partial_channel& channel, snowflake id, int n) {
	nlohmann::json body;
	body["limit"] = n;
	body["around"] = id;
	return m_send_things<rq::get_messages>(body.dump(), channel);
}

rq::get_messages shard::get_messages_before(const partial_channel& channel, const partial_message& msg, int n) {
	nlohmann::json body;
	body["limit"] = n;
	body["around"] = msg.id();
	return m_send_things<rq::get_messages>(body.dump(), channel);
}

rq::get_messages shard::get_messages_after(const partial_channel& channel, const partial_message& msg, int n) {
	nlohmann::json body;
	body["limit"] = n;
	body["around"] = msg.id();
	return m_send_things<rq::get_messages>(body.dump(), channel);
}

rq::get_messages shard::get_messages_around(const partial_channel& channel, const partial_message& msg, int n) {
	nlohmann::json body;
	body["limit"] = n;
	body["around"] = msg.id();
	return m_send_things<rq::get_messages>(body.dump(), channel);
}

rq::create_text_channel shard::create_text_channel(const Guild& g, std::string name, std::vector<permission_overwrite> permission_overwrites, bool nsfw) {
	nlohmann::json body;
	body["type"] = 0;
	body["name"] = std::move(name);
	body["permission_overwrites"] = std::move(permission_overwrites);
	body["nsfw"] = nsfw;
	return m_send_things<rq::create_text_channel>(body.dump(),g);
}

rq::edit_message shard::edit_message(const partial_message& msg, std::string new_content) {
	nlohmann::json body;
	body["content"] = std::move(new_content);
	return m_send_things<rq::edit_message>(body.dump(), msg);
}

rq::create_voice_channel shard::create_voice_channel(const Guild& g, std::string name, std::vector<permission_overwrite> permission_overwrites,bool nsfw, int bit_rate) {
	nlohmann::json body;
	body["nsfw"] = nsfw;
	body["name"] = std::move(name);
	body["permission_overwrites"] = std::move(permission_overwrites);
	body["bit_rate"] = bit_rate;
	return m_send_things<rq::create_voice_channel>(body.dump(),g);
}

rq::create_channel_catagory shard::create_channel_catagory(const Guild& g, std::string name,std::vector<permission_overwrite> permission_overwrite, bool nsfw) {
	nlohmann::json body;
	body["nsfw"] = nsfw;
	body["name"] = std::move(name);
	body["permission_overwrites"] = std::move(permission_overwrite);
	return m_send_things<rq::create_channel_catagory>(body.dump(),g);	
}

rq::delete_message shard::delete_message(const partial_message& msg) {
	return m_send_things<rq::delete_message>(msg);
}

rq::delete_message_bulk shard::delete_message_bulk(const partial_channel& channel, const std::vector<partial_message>& msgs){
	nlohmann::json body;
	std::vector<snowflake> things(msgs.size());
	std::transform(msgs.begin(), msgs.end(), things.begin(), [](const auto& msg) {return msg.id(); });
	body["messages"] = std::move(things);
	return m_send_things<rq::delete_message_bulk>(body.dump(), channel);
}

rq::delete_message_bulk shard::delete_message_bulk(const partial_channel& channel, std::vector<snowflake> msgs){
	nlohmann::json body;
	body["messages"] = std::move(msgs);
	return m_send_things<rq::delete_message_bulk>(body.dump(),channel);
}

rq::delete_emoji shard::delete_emoji(const partial_guild& g, const emoji& e) {
	return m_send_things<rq::delete_emoji>(g, e);
}

rq::leave_guild shard::leave_guild(const Guild& g) {
	return m_send_things<rq::leave_guild>(g);
}

rq::add_reaction shard::add_reaction(const partial_message& msg, const partial_emoji& e) {
	return m_send_things<rq::add_reaction>(msg, e);
}

rq::typing_start shard::typing_start(const partial_channel& ch) {
	return m_send_things<rq::typing_start>(ch);
}

rq::delete_channel_permission shard::delete_channel_permissions(const guild_channel& a,const permission_overwrite& b) {
	return m_send_things<rq::delete_channel_permission>(a, b);
}

rq::modify_channel_positions shard::modify_channel_positions(const Guild& g, const std::vector<std::pair<snowflake, int>>&) {
	return m_send_things<rq::modify_channel_positions>(""s, g);
}

rq::list_guild_members shard::list_guild_members(const partial_guild& g, int n, snowflake after) {
	nlohmann::json body;
	body["limit"] = n;
	body["after"] = after;
	return m_send_things<rq::list_guild_members>(body.dump(),g);
}

rq::edit_channel_permissions shard::edit_channel_permissions(const guild_channel& ch, const permission_overwrite& overwrite) {
	nlohmann::json body = { {"allow",overwrite.allow()},{"deny",overwrite.deny()},{"type",overwrite_type_to_string(overwrite.type())} };
	return m_send_things<rq::edit_channel_permissions>(body.dump(), ch, overwrite);
}

rq::create_dm shard::create_dm(const user& user) {
	nlohmann::json body;
	body["recipient_id"] = user.id();
	return m_send_things<rq::create_dm>(body.dump());
}

rq::delete_channel shard::delete_channel(const partial_channel& ch) {
	return m_send_things<rq::delete_channel>(ch);
}

rq::add_pinned_msg shard::add_pinned_msg(const partial_channel& ch, const partial_message& msg) {
	return m_send_things<rq::add_pinned_msg>(ch, msg);
}

rq::remove_pinned_msg shard::remove_pinned_msg(const partial_channel& ch, const partial_message& msg) {
	return m_send_things<rq::remove_pinned_msg>(ch, msg);
}

rq::get_voice_regions shard::get_voice_regions() {
	return m_send_things<rq::get_voice_regions>();
}

rq::create_channel_invite shard::create_channel_invite(const guild_channel& ch, int max_age, int max_uses, bool temporary,	bool unique) {
	nlohmann::json body;
	body["max_age"] = max_age;
	body["max_uses"] = max_uses;
	body["temporary"] = temporary;
	body["unique"] = unique;
	return m_send_things<rq::create_channel_invite>(body.dump(), ch);
}
