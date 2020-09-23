#include "shard.h"
#include "client.h"
#include "internal_shard.h"
#include "../common/common/optional_from_json.h"

namespace cacheless {

shard::shard(int shard_number, client* t_parent, boost::asio::io_context& ioc, std::string auth_token) :
	m_shard_number(shard_number),
	m_http_connection(t_parent, ioc),
	m_strand(ioc),
	m_parent_client(t_parent),
	m_auth_token(std::move(auth_token)) { }

bool shard::will_have_guild(snowflake guild_id) const noexcept {
	return (guild_id.val >> 22) % m_parent_client->num_shards() == m_shard_number;
}

// cerwy::task<boost::beast::error_code> shard::connect_http_connection() {
// 	auto ec = co_await m_http_connection.async_connect();
// 	int tries = 1;
// 	//TODO: change this
// 	while (ec && tries < 10) {
// 		ec = co_await m_http_connection.async_connect();
// 		++tries;
// 	}
// 	co_return ec;
// }

void shard::set_up_request(boost::beast::http::request<boost::beast::http::string_body>& req) const {
	req.set("Application", "cerwy");
	req.set(boost::beast::http::field::authorization, m_auth_token);
	req.set(boost::beast::http::field::host, "discord.com"s);
	req.set(boost::beast::http::field::user_agent, "watland");
	req.set(boost::beast::http::field::accept, "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	req.set(boost::beast::http::field::accept_language, "en-US,en;q=0.5");
	req.keep_alive(true);
	//m_parent->set_up_request(req);
}

rq::send_message shard::send_message(snowflake channel, std::string content) {
	nlohmann::json body;
	body["content"] = std::move(content);
	return send_request<rq::send_message>(body.dump(), channel);
}

rq::send_message shard::send_message(snowflake channel, std::string content, const embed& embed) {
	nlohmann::json body;
	body["content"] = std::move(content);
	body["embed"] = embed;
	return send_request<rq::send_message>(body.dump(), channel);
}

rq::add_role shard::add_role(snowflake guild, snowflake member, snowflake role) {
	return send_request<rq::add_role>(guild, member, role);
}

rq::remove_role shard::remove_role(snowflake guild, snowflake member, snowflake role) {
	return send_request<rq::remove_role>(guild, member, role);
}

rq::modify_member shard::remove_all_roles(snowflake guild, snowflake member) {
	return send_request<rq::modify_member>(R"({"roles":[]})"s, guild, member);
}

rq::create_role shard::create_role(snowflake g, std::string name, permission p, int color, bool hoist, bool mentionable) {
	nlohmann::json body;
	body["name"] = std::move(name);
	body["permissions"] = p;
	body["color"] = color;
	body["hoist"] = hoist;
	body["mentionable"] = mentionable;
	return send_request<rq::create_role>(body.dump(), g);
}

rq::delete_role shard::delete_role(snowflake g, snowflake role_id) {
	return send_request<rq::delete_role>(g, role_id);
}

rq::modify_member shard::change_nick(snowflake g, snowflake member_id, std::string new_nick) {
	nlohmann::json body;
	body["nick"] = std::move(new_nick);
	return send_request<rq::modify_member>(body.dump(), g, member_id);
}

rq::change_my_nick shard::change_my_nick(snowflake g, std::string new_nick) {
	nlohmann::json body;
	body["nick"] = std::move(new_nick);
	return send_request<rq::change_my_nick>(body.dump(), g);
}

rq::kick_member shard::kick_member(snowflake g, snowflake member_id) {
	return send_request<rq::kick_member>(g, member_id);
}

rq::ban_member shard::ban_member(snowflake g, snowflake member_id, std::string reason, int days_to_delete_msg) {
	nlohmann::json body;
	body["delete-message-days"] = days_to_delete_msg;
	body["reason"] = std::move(reason);
	return send_request<rq::ban_member>(body.dump(), g, member_id);
}

rq::unban_member shard::unban_member(snowflake g, snowflake member_id) {
	return send_request<rq::unban_member>(g, member_id);
}

rq::get_messages shard::get_messages(snowflake channel_id, int n) {
	nlohmann::json body;
	body["limit"] = n;
	return send_request<rq::get_messages>(body.dump(), channel_id);
}

rq::get_messages shard::get_messages_before(snowflake channel_id, snowflake id, int n) {
	nlohmann::json body;
	body["limit"] = n;
	body["before"] = id;
	return send_request<rq::get_messages>(body.dump(), channel_id);
}

rq::get_messages shard::get_messages_after(snowflake channel_id, snowflake id, int n) {
	nlohmann::json body;
	body["limit"] = n;
	body["after"] = id;
	return send_request<rq::get_messages>(body.dump(), channel_id);
}

rq::get_messages shard::get_messages_around(snowflake channel_id, snowflake id, int n) {
	nlohmann::json body;
	body["limit"] = n;
	body["around"] = id;
	return send_request<rq::get_messages>(body.dump(), channel_id);
}

rq::create_text_channel shard::create_text_channel(snowflake g, std::string name, std::vector<permission_overwrite> permission_overwrites, bool nsfw) {
	nlohmann::json body;
	body["type"] = 0;
	body["name"] = std::move(name);
	body["permission_overwrites"] = std::move(permission_overwrites);
	body["nsfw"] = nsfw;
	return send_request<rq::create_text_channel>(body.dump(), g);
}

rq::edit_message shard::edit_message(snowflake channel_id, snowflake msg_id, std::string new_content) {
	nlohmann::json body;
	body["content"] = std::move(new_content);
	return send_request<rq::edit_message>(body.dump(), channel_id, msg_id);
}

rq::create_voice_channel shard::create_voice_channel(snowflake g, std::string name, std::vector<permission_overwrite> permission_overwrites, bool nsfw, int bit_rate) {
	nlohmann::json body;
	body["nsfw"] = nsfw;
	body["name"] = std::move(name);
	body["permission_overwrites"] = std::move(permission_overwrites);
	body["bit_rate"] = bit_rate;
	return send_request<rq::create_voice_channel>(body.dump(), g);
}

rq::create_channel_catagory shard::create_channel_catagory(snowflake g, std::string name, std::vector<permission_overwrite> permission_overwrite, bool nsfw) {
	nlohmann::json body;
	body["nsfw"] = nsfw;
	body["name"] = std::move(name);
	body["permission_overwrites"] = std::move(permission_overwrite);
	return send_request<rq::create_channel_catagory>(body.dump(), g);
}

rq::delete_message shard::delete_message(snowflake channel_id, snowflake msg_id) {
	return send_request<rq::delete_message>(channel_id, msg_id);
}

rq::delete_message_bulk shard::delete_message_bulk(snowflake channel_id, std::vector<snowflake> msgs) {
	nlohmann::json body;
	body["messages"] = std::move(msgs);
	return send_request<rq::delete_message_bulk>(body.dump(), channel_id);
}

rq::delete_emoji shard::delete_emoji(snowflake g, const partial_emoji& e) {
	return send_request<rq::delete_emoji>(g, e);
}

rq::modify_emoji shard::modify_emoji(snowflake g, const partial_emoji& e, std::string s, std::vector<snowflake> v) {
	nlohmann::json body;
	body["name"] = std::move(s);
	body["roles"] = std::move(v);
	return send_request<rq::modify_emoji>(body.dump(), g, e);
}

rq::leave_guild shard::leave_guild(snowflake g) {
	return send_request<rq::leave_guild>(g);
}

rq::add_reaction shard::add_reaction(snowflake channel_id, snowflake msg_id, const partial_emoji& e) {
	return send_request<rq::add_reaction>(channel_id, msg_id, e);
}

rq::delete_own_reaction shard::delete_own_reaction(snowflake channel_id, snowflake msg_id, const partial_emoji& emoji) {
	return send_request<rq::delete_own_reaction>(channel_id, msg_id, emoji);
}

rq::delete_own_reaction shard::delete_own_reaction(snowflake channel_id, snowflake msg_id, const reaction& rc) {
	return send_request<rq::delete_own_reaction>(channel_id, msg_id, rc);
}

rq::delete_user_reaction shard::delete_user_reaction(snowflake channel_id, snowflake msg_id, const partial_emoji& emoji, const snowflake& user) {
	return send_request<rq::delete_user_reaction>(channel_id, msg_id, emoji, user);
}

rq::delete_user_reaction shard::delete_user_reaction(snowflake channel_id, snowflake msg_id, const reaction& reaction, const snowflake& user) {
	return send_request<rq::delete_user_reaction>(channel_id, msg_id, reaction, user);
}

rq::get_reactions shard::get_reactions(snowflake channel_id, snowflake msg_id, const partial_emoji& emoji) {
	return send_request<rq::get_reactions>(channel_id, msg_id, emoji);
}

rq::get_reactions shard::get_reactions(snowflake channel_id, snowflake msg_id, const reaction& rc) {
	return send_request<rq::get_reactions>(channel_id, msg_id, rc);
}

rq::delete_all_reactions shard::delete_all_reactions(snowflake channel_id, snowflake msg_id) {
	return send_request<rq::delete_all_reactions>(channel_id, msg_id);
}

rq::delete_all_reactions_emoji shard::delete_all_reactions_emoji(snowflake channel_id, snowflake msg_id, const partial_emoji& emoji) {
	return send_request<rq::delete_all_reactions_emoji>(channel_id, msg_id, emoji);
}

rq::delete_all_reactions_emoji shard::delete_all_reactions_emoji(snowflake channel_id, snowflake msg_id, const reaction& rc) {
	return send_request<rq::delete_all_reactions_emoji>(channel_id,msg_id, rc);
}

rq::typing_start shard::typing_start(snowflake ch) {
	return send_request<rq::typing_start>(ch);
}

rq::delete_channel_permission shard::delete_channel_permissions(snowflake channel_id, const permission_overwrite& b) {
	return send_request<rq::delete_channel_permission>(channel_id, b);
}

rq::list_guild_members shard::list_guild_members(snowflake g, int n, snowflake after) {
	nlohmann::json body;
	body["limit"] = n;
	body["after"] = after;
	return send_request<rq::list_guild_members>(body.dump(), g);
}

rq::edit_channel_permissions shard::edit_channel_permissions(snowflake ch, const permission_overwrite& overwrite) {
	nlohmann::json body = {{"allow", overwrite.allow}, {"deny", overwrite.deny}, {"type", overwrite_type_to_string(overwrite.type)}};
	return send_request<rq::edit_channel_permissions>(body.dump(), ch, overwrite);
}

rq::create_dm shard::create_dm(const user& user) {
	nlohmann::json body;
	body["recipient_id"] = user.id;
	return send_request<rq::create_dm>(body.dump());
}

rq::get_guild_integrations shard::get_guild_integrations(snowflake g) {
	return send_request<rq::get_guild_integrations>(g);
}

rq::create_guild_integration shard::create_guild_integration(snowflake g, std::string type, snowflake id) {
	nlohmann::json body;
	body["type"] = type;
	body["id"] = id;
	return send_request<rq::create_guild_integration>(body.dump(), g);
}

rq::delete_channel shard::delete_channel(snowflake channel_id) {
	return send_request<rq::delete_channel>(channel_id);
}

rq::add_pinned_msg shard::add_pinned_msg(snowflake channel_id, snowflake msg_id) {
	return send_request<rq::add_pinned_msg>(channel_id, msg_id);
}

rq::remove_pinned_msg shard::remove_pinned_msg(snowflake channel_id, snowflake msg_id) {
	return send_request<rq::remove_pinned_msg>(channel_id, msg_id);
}

rq::get_voice_regions shard::get_voice_regions() {
	return send_request<rq::get_voice_regions>();
}

rq::create_channel_invite shard::create_channel_invite(snowflake channel_id, int max_age, int max_uses, bool temporary, bool unique) {
	nlohmann::json body;
	body["max_age"] = max_age;
	body["max_uses"] = max_uses;
	body["temporary"] = temporary;
	body["unique"] = unique;
	return send_request<rq::create_channel_invite>(body.dump(), channel_id);
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

rq::create_webhook shard::create_webhook(snowflake channel_id, std::string name) {
	nlohmann::json json;
	json["name"] = std::move(name);
	json["avatar"] = nlohmann::json();//null
	return send_request<rq::create_webhook>(json.dump(), channel_id);
}

rq::get_guild_webhooks shard::get_guild_webhooks(snowflake guild_id) {
	return send_request<rq::get_guild_webhooks>(guild_id);
}

rq::get_channel_webhooks shard::get_channel_webhooks(snowflake channel_id) {
	return send_request<rq::get_channel_webhooks>(channel_id);
}

rq::get_webhook shard::get_webhook(snowflake id, std::string token) {
	return send_request<rq::get_webhook>(id, token);
}

rq::get_webhook shard::get_webhook(const webhook& wh) {
	return send_request<rq::get_webhook>(wh.id);
}

rq::execute_webhook shard::send_with_webhook(const webhook& wh, std::string s) {
	nlohmann::json json;
	json["content"] = std::move(s);
	return send_request<rq::execute_webhook>(json.dump(), wh);
}

rq::get_message shard::fetch_message(snowflake channel_id, snowflake msg_id) {
	return send_request<rq::get_message>(channel_id, msg_id);
}

rq::get_audit_log shard::get_audit_log(snowflake guild_id) {
	return send_request<rq::get_audit_log>(guild_id);
}


}
