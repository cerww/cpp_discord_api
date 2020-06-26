#include "shard.h"
#include "client.h"
#include "internal_shard.h"


shard::shard(int shard_number, client* t_parent, boost::asio::io_context& ioc) :
	m_shard_number(shard_number),
	m_parent(t_parent),
	m_strand(ioc),
	m_http_connection(t_parent, ioc) { }

cerwy::task<voice_connection> shard::connect_voice(const voice_channel& ch) {
	//;-;
	//TODO: change this
	auto& me = (internal_shard&)*this;
	return me.connect_voice(ch);
<<<<<<< HEAD
}

bool shard::will_have_guild(snowflake guild_id) const noexcept{
	return (guild_id.val >> 22) % m_parent->num_shards() == m_shard_number;
=======
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
}

cerwy::task<boost::beast::error_code> shard::connect_http_connection() {
	auto ec = co_await m_http_connection.async_connect();
	int tries = 1;
<<<<<<< HEAD
	//TODO: change this
=======
>>>>>>> 9648113a4d7aa9623d8a04cb8224e805b3cf95de
	while (ec && tries < 10) {
		ec = co_await m_http_connection.async_connect();
		++tries;
	}
	co_return ec;
}

void shard::set_up_request(boost::beast::http::request<boost::beast::http::string_body>& req) const {
	m_parent->set_up_request(req);
}

rq::send_message shard::send_message(const text_channel& channel, std::string content) {
	nlohmann::json body;
	body["content"] = std::move(content);
	return send_request<rq::send_message>(body.dump(), channel);
}

rq::send_message shard::send_message(const dm_channel& channel, std::string content) {
	nlohmann::json body;
	body["content"] = std::move(content);
	return send_request<rq::send_message>(body.dump(), channel);
}

rq::add_role shard::add_role(const partial_guild& guild, const guild_member& member, const guild_role& role) {
	return send_request<rq::add_role>(guild, member, role);
}

rq::remove_role shard::remove_role(const partial_guild& guild, const guild_member& member, const guild_role& role) {
	return send_request<rq::remove_role>(guild, member, role);
}

rq::modify_member shard::remove_all_roles(const partial_guild& guild, const guild_member& member) {
	return send_request<rq::modify_member>(R"({"roles":[]})"s, guild, member);
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

rq::kick_member shard::kick_member(const partial_guild& g, const guild_member& member) {
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
	return send_request<rq::create_text_channel>(body.dump(), g);
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
	return send_request<rq::create_voice_channel>(body.dump(), g);
}

rq::create_channel_catagory shard::create_channel_catagory(const Guild& g, std::string name, std::vector<permission_overwrite> permission_overwrite, bool nsfw) {
	nlohmann::json body;
	body["nsfw"] = nsfw;
	body["name"] = std::move(name);
	body["permission_overwrites"] = std::move(permission_overwrite);
	return send_request<rq::create_channel_catagory>(body.dump(), g);
}

rq::delete_message shard::delete_message(const partial_message& msg) {
	return send_request<rq::delete_message>(msg);
}

rq::delete_message_bulk shard::delete_message_bulk(const partial_channel& channel, std::vector<snowflake> msgs) {
	nlohmann::json body;
	body["messages"] = std::move(msgs);
	return send_request<rq::delete_message_bulk>(body.dump(), channel);
}

rq::delete_emoji shard::delete_emoji(const partial_guild& g, const emoji& e) {
	return send_request<rq::delete_emoji>(g, e);
}

rq::modify_emoji shard::modify_emoji(Guild& g, emoji& e, std::string s, std::vector<snowflake> v) {
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

rq::delete_channel_permission shard::delete_channel_permissions(const partial_guild_channel& a, const permission_overwrite& b) {
	return send_request<rq::delete_channel_permission>(a, b);
}

rq::list_guild_members shard::list_guild_members(const partial_guild& g, int n, snowflake after) {
	nlohmann::json body;
	body["limit"] = n;
	body["after"] = after;
	return send_request<rq::list_guild_members>(body.dump(), g);
}

rq::edit_channel_permissions shard::edit_channel_permissions(const partial_guild_channel& ch, const permission_overwrite& overwrite) {
	nlohmann::json body = {{"allow", overwrite.allow()}, {"deny", overwrite.deny()}, {"type", overwrite_type_to_string(overwrite.type())}};
	return send_request<rq::edit_channel_permissions>(body.dump(), ch, overwrite);
}

rq::create_dm shard::create_dm(const user& user) {
	nlohmann::json body;
	body["recipient_id"] = user.id();
	return send_request<rq::create_dm>(body.dump());
}

rq::get_guild_integrations shard::get_guild_integrations(const partial_guild& g) {
	return send_request<rq::get_guild_integrations>(g);
}

rq::create_guild_integration shard::create_guild_integration(const partial_guild& g, std::string type, snowflake id) {
	nlohmann::json body;
	body["type"] = type;
	body["id"] = id;
	return send_request<rq::create_guild_integration>(body.dump(), g);
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

rq::create_channel_invite shard::create_channel_invite(const partial_guild_channel& ch, int max_age, int max_uses, bool temporary, bool unique) {
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
	} else {
		return send_request<rq::get_invite>("", s);
	}
}

rq::delete_invite shard::delete_invite(std::string s) {
	return send_request<rq::delete_invite>(s);
}
