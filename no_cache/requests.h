#pragma once
#include <fmt/core.h>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>
#include "guild.h"
#include "../common/ref_count_ptr.h"
#include "voice_state.h"
#include "invite.h"
#include <boost/asio.hpp>
#include <range/v3/all.hpp>
#include "guild_integration.h"
#include "webhook.h"
#include "audit_log.h"
#include "../common/crtp_stuff.h"
#include <coroutine>

// clang-format off
namespace cacheless::rq {
	
//

struct bad_request final :std::exception {
	using exception::exception;
};

struct unauthorized final :std::exception {
	using exception::exception;
};

struct forbidden final :std::exception {
	using exception::exception;
};

struct not_found final :std::exception {
	using exception::exception;
};

struct method_not_allowed final :std::exception {
	using exception::exception;
};

struct gateway_unavailable final :std::exception {
	using exception::exception;

	const char* what() const final {
		return "gateway unavailible try again";
	}
};

struct server_error final :std::exception {
	using exception::exception;

	const char* what() const final {
		return "server error";
	}
};

struct shared_state :ref_counted {
	//rename some of these.
	std::atomic<bool> done = false;
	boost::asio::io_context::strand* strand = nullptr;
	std::vector<std::coroutine_handle<>> waiters{};
	std::mutex ready_mut{};
	std::mutex waiter_mut{};
	std::condition_variable ready_cv{};
	boost::beast::http::response<boost::beast::http::string_body> res = {};
	std::optional<std::exception_ptr> exception;//rename?

	void notify() {
		ready_cv.notify_all();

		std::lock_guard lock(waiter_mut);
		auto all_waiters = std::move(waiters);

		const auto execute_on_strand = [&](auto&& f) {
			boost::asio::post(*strand, f);
		};

		ranges::for_each(all_waiters, execute_on_strand);
	}
};

constexpr static int ajkdhas = sizeof(std::condition_variable);

template<typename U, typename = void>
struct has_overload_value :std::false_type {};

template<typename U>
struct has_overload_value<U, std::void_t<decltype(std::declval<U>().overload_value())>> :std::true_type {};

static constexpr const char* application_json = "application/json";

struct json_content_type {
	static constexpr const char* content_type = application_json;
};

struct delete_verb {
	static constexpr auto verb = boost::beast::http::verb::delete_;
};

struct put_verb {
	static constexpr auto verb = boost::beast::http::verb::put;
};

struct get_verb {
	static constexpr auto verb = boost::beast::http::verb::get;
};

struct post_verb {
	static constexpr auto verb = boost::beast::http::verb::post;
};

struct patch_verb {
	static constexpr auto verb = boost::beast::http::verb::patch;
};

template<typename T>
struct with_exception {
	T exception;
};

template<typename reqeust>
struct request_base :private crtp<reqeust> {
	request_base() = default;

	explicit request_base(ref_count_ptr<shared_state> t_state):
		state(std::move(t_state)) {}

	template<typename T>
	explicit request_base(with_exception<T> e, boost::asio::io_context::strand* strand):
		request_base(make_ref_count_ptr<shared_state>()) {
		state->strand = strand;
		state->done = true;
		state->exception = std::make_exception_ptr(e.exception);
	}

	template<typename ...Ts>
	static auto request(Ts&&... t) {
		return boost::beast::http::request<boost::beast::http::string_body>(
			reqeust::verb,
			"/api/v6" + reqeust::target(std::forward<Ts>(t)...),
			11
		);
	}

	void handle_errors() const {
		if (state->exception.has_value()) {
			std::rethrow_exception(state->exception.value());
		}
		const auto status = state->res.result_int();
		if (status == 400)
			throw bad_request(";-;");
		if (status == 401)
			throw unauthorized();
		if (status == 403)
			throw forbidden();
		if (status == 404)
			throw not_found();
		if (status == 405)
			throw method_not_allowed();
		if (status == 502)
			throw gateway_unavailable();
		if (status >= 500)
			throw server_error();
	}

	void wait() {
		std::unique_lock<std::mutex> lock{state->ready_mut};
		state->ready_cv.wait(lock, [this]()->bool { return ready(); });
	}

	bool ready() const noexcept {
		return state->done.load(std::memory_order_relaxed);
	}

	bool await_ready() const noexcept {
		return false;//await suspend checks instead
	}

	void await_suspend(std::coroutine_handle<> h) const {		
		std::unique_lock lock(state->waiter_mut);
		if (ready()) {
			lock.unlock();
			h.resume();
		} else {
			state->waiters.push_back(h);
		}
	}

	decltype(auto) await_resume() const {
		if constexpr (!std::is_void_v<typename reqeust::return_type>)
			return value();
	}

	decltype(auto) value() const {
		handle_errors();
		if constexpr (has_overload_value<reqeust>::value)
			return this->self().overload_value();
		else if constexpr (!std::is_void_v<typename reqeust::return_type>)
			return nlohmann::json::parse(state->res.body()).template get<typename reqeust::return_type>();
	}
		
	//remove along with wait?
	decltype(auto) get() {
		wait();
		handle_errors();
		if constexpr (!std::is_void_v<typename reqeust::return_type>)
			return value();
	}
		
	//returns awaitable and ignores the value returned from discord's api
	auto async_wait() {
		struct rabc {
			request_base* me = nullptr;

			bool await_ready() {
				return me->await_ready();
			}

			decltype(auto) await_suspend(std::coroutine_handle<> h) {
				me->await_suspend(h);
			}

			decltype(auto) await_resume() {
				//me->handle_errors();
				//return me->await_resume();
			}

		};
		return rabc{this};
	}

protected:
	ref_count_ptr<shared_state> state;
};

struct get_guild :
		request_base<get_guild>,
		get_verb {
	using request_base::request_base;
	using return_type = partial_guild;

	static std::string target(snowflake id) {

		return fmt::format("/guilds/{}", id);
	}
};

struct send_message :
		request_base<send_message>,
		json_content_type,
		post_verb {

	using request_base::request_base;
	using return_type = partial_message;

	static std::string target(snowflake channel_id) {

		return fmt::format("/channels/{}/messages", channel_id);
	}
};

struct send_message_with_file :
		request_base<send_message_with_file>,
		json_content_type,
		post_verb {
	using request_base::request_base;
	using return_type = partial_message;

	static std::string target(snowflake channel) {

		return fmt::format("/channels/{}/messages", channel);
	}
};

struct add_role :
		request_base<add_role>,
		put_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake g, snowflake m, snowflake r) {

		return fmt::format("/guilds/{}/members/{}/roles/{}", g, m, r);
	}
};

struct remove_role :
		request_base<remove_role>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake g, snowflake m, snowflake r) {

		return fmt::format("/guilds/{}/members/{}/roles/{}", g, m, r);
	}
};

struct create_role :
		request_base<create_role>,
		post_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = guild_role;

	static std::string target(snowflake g) {

		return fmt::format("/guilds/{}/roles", g);
	}
};

struct delete_role :
		request_base<delete_role>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static auto target(snowflake g, snowflake r) {

		return fmt::format("/guilds/{}/roles/{}", g, r);
	}
};

struct modify_role :
		request_base<modify_role>,
		json_content_type,
		patch_verb {
	using request_base::request_base;
	using return_type = guild_role;

	static std::string target(snowflake g, snowflake r) {

		return fmt::format("/guilds/{}/roles/{}", g, r);
	}
};

struct kick_member :
		request_base<kick_member>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake g, snowflake member) {

		return fmt::format("/guilds/{}/members/{}", g, member);
	}
};

struct ban_member :
		request_base<ban_member>,
		json_content_type,
		put_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake g, snowflake member) {

		return fmt::format("/guilds/{}/bans/{}", g, member);
	}
};

struct modify_member :
		request_base<modify_member>,
		json_content_type,
		patch_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake g, snowflake member) {

		return fmt::format("/guilds/{}/members/{}", g, member);
	}
};

struct change_my_nick :
		request_base<change_my_nick>,
		json_content_type,
		patch_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake g) {

		return fmt::format("/guilds/{}/members/@me/nick", g);
	}
};

struct delete_message :
		request_base<delete_message>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake channel_id, snowflake msg) {

		return fmt::format("/channels/{}/messages/{}", channel_id, msg);
	}
};

struct edit_message :
		request_base<edit_message>,
		patch_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = partial_message;

	static std::string target(snowflake channel_id, snowflake msg) {

		return fmt::format("/channels/{}/messages/{}", channel_id, msg);
	}
};

struct get_messages :
		request_base<get_messages>,
		json_content_type,
		get_verb {
	using request_base::request_base;
	using return_type = std::vector<partial_message>;

	static std::string target(snowflake channel) {

		return fmt::format("/channels/{}/messages", channel);
	}
};

struct unban_member :
		request_base<unban_member>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake guild, snowflake user_id) {

		return fmt::format("/guilds/{}/bans/{}", guild, user_id);
	}
};

struct create_text_channel :
		request_base<create_text_channel>,
		json_content_type,
		post_verb {

	using request_base::request_base;
	using return_type = partial_guild_channel;

	static std::string target(snowflake guild) {

		return fmt::format("/guilds/{}/channels", guild);
	}
	/*
	snowflake overload_value() const{
		return nlohmann::json::parse(state->res.body())["id"].get<snowflake>();
	}

	//*/
};

struct create_voice_channel :
		request_base<create_voice_channel>,
		post_verb,
		json_content_type {
	using request_base::request_base;
	//using return_type = snowflake;
	using return_type = partial_guild_channel;

	static std::string target(snowflake guild) {

		return fmt::format("/guilds/{}/channels", guild);
	}
	/*snowflake overload_value() const{
		return nlohmann::json::parse(state->res.body())["id"].get<snowflake>();
	}*/
};

struct create_channel_catagory :
		request_base<create_channel_catagory>,
		post_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = partial_guild_channel;

	static std::string target(snowflake guild) {

		return fmt::format("/guilds/{}/channels", guild);
	}
};

struct delete_emoji :
		request_base<delete_emoji>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake guild, const partial_emoji& e) {

		return fmt::format("/guilds/{}/emojis/{}", guild, e.id);
	}
};

struct modify_emoji :
		request_base<modify_emoji>,
		json_content_type,
		patch_verb {
	using request_base::request_base;
	using return_type = emoji;

	static std::string target(snowflake guild, const partial_emoji& e) {

		return fmt::format("/guilds/{}/emojis/{}", guild, e.id);
	}
};

struct delete_message_bulk :
		request_base<delete_message_bulk>,
		post_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake channel) {

		return fmt::format("/channels/{}/messages/bulk-delete", channel);
	}
};

struct leave_guild :
		request_base<leave_guild>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake g) {

		return fmt::format("/@me/guilds/{}", g);
	}
};

struct get_voice_regions :
		request_base<get_voice_regions> {
	using request_base::request_base;
	using return_type = std::vector<voice_region>;

	static constexpr auto verb = boost::beast::http::verb::get;

	static std::string target() {
		return fmt::format("/voice/regions");
	}
};

struct current_guilds :
		request_base<current_guilds>,
		get_verb {
	using request_base::request_base;
	using return_type = std::vector<partial_guild>;

	static std::string target() {
		return fmt::format("/users/@me/guilds");
	}
};

struct add_reaction :
		request_base<add_reaction>,
		put_verb {
	using request_base::request_base;
	using return_type = void;


	static std::string target(snowflake channel_id, snowflake msg, const partial_emoji& emo) {

		return fmt::format("/channels/{}/messages/{}/reactions/{}/@me", channel_id, msg, emo.to_reaction_string());
	}
};

struct delete_own_reaction :
		request_base<delete_own_reaction>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake channel_id, snowflake msg, const partial_emoji& emoji) {

		return fmt::format("/channels/{}/messages/{}/reactions/{}/@me", channel_id, msg, emoji.to_reaction_string());
	}

	static std::string target(snowflake channel_id, snowflake msg, const reaction& reaction) {
		return target(channel_id, msg, reaction.emoji);
	}

};

struct delete_user_reaction :
		request_base<delete_user_reaction>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake channel_id, snowflake msg, const partial_emoji& emoji, snowflake member) {

		return fmt::format("/channels/{}/messages/{}/reactions/{}/{}", channel_id, msg, emoji.to_reaction_string(), member);
	}

	static std::string target(snowflake channel_id, snowflake msg, const reaction& react, snowflake member) {
		return target(channel_id, msg, react.emoji, member);
	}
};

struct get_reactions :
		request_base<get_reactions>,
		get_verb {

	using request_base::request_base;
	using return_type = std::vector<user>;

	static std::string target(snowflake channel_id, snowflake msg, const partial_emoji& emoji) {

		return fmt::format("/channels/{}/messages/{}/reactions/{}", channel_id, msg, emoji.to_reaction_string());
	}

	static std::string target(snowflake channel_id, snowflake msg, const reaction& emoji) {

		return fmt::format("/channels/{}/messages/{}/reactions/{}", channel_id, msg, emoji.emoji.to_reaction_string());
	}

};

struct delete_all_reactions :
		request_base<delete_all_reactions>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake channel_id, snowflake msg) {

		return fmt::format("/channels/{}/messages/{}/reactions", channel_id, msg);
	}
};

struct delete_all_reactions_emoji :
		request_base<delete_all_reactions_emoji>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake channel_id, snowflake msg, const partial_emoji& emoji) {

		return fmt::format("/channels/{}/messages/{}/reactions/{}", channel_id, msg, emoji.to_reaction_string());
	}

	static std::string target(snowflake channel_id, snowflake msg, const reaction& reaction) {
		return target(channel_id, msg, reaction.emoji);;
	}

};

struct typing_start :
		request_base<typing_start>,
		post_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake ch) {

		return fmt::format("/channels/{}/typing", ch);
	}
};

struct delete_channel_permission :
		request_base<delete_channel_permission>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake ch, const permission_overwrite& o) {

		return fmt::format("/channels/{}/permissions/{}", ch, o.id);
	}
};

struct modify_guild :
		request_base<modify_guild>,
		patch_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = partial_guild;

	static std::string target(snowflake g) {

		return fmt::format("/guild/{}", g);
	}
};

struct modify_channel_positions :
		request_base<modify_channel_positions>,
		patch_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake g) {

		return fmt::format("/guilds/{}/channels", g);
	}
};

struct add_guild_member :
		request_base<add_guild_member>,
		put_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = snowflake;

	static std::string target(snowflake g, snowflake id) {

		return fmt::format("/guilds/{}/members/{}", g, id);
	}

	snowflake overload_value() const {
		return nlohmann::json::parse(state->res.body())["id"].get<snowflake>();
	}
};

struct delete_channel :
		request_base<delete_channel>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake ch) {

		return fmt::format("/channels/{}", ch);
	}
};

struct add_pinned_msg :
		request_base<add_pinned_msg>,
		put_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake ch, snowflake msg) {

		return fmt::format("/channels/{}/pins/{}", ch, msg);
	}
};

struct remove_pinned_msg :
		request_base<remove_pinned_msg>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake ch, snowflake msg) {

		return fmt::format("/channels/{}/pins/{}", ch, msg);
	}
};

struct list_guild_members :
		request_base<list_guild_members>,
		get_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = std::vector<guild_member>;

	static std::string target(snowflake g) {

		return fmt::format("/guilds/{}/members", g);
	}

};

struct edit_channel_permissions :
		request_base<edit_channel_permissions>,
		put_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake c, const permission_overwrite& p) {

		return fmt::format("/channels/{}/permissions/{}", c, p.id);
	};
};

struct create_dm :
		request_base<create_dm>,
		post_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = partial_channel;
	// using return_type = snowflake;

	static std::string target() {
		return fmt::format("/users/@me/channels");
	}

	// return_type overload_value() const{
	// 	return nlohmann::json::parse(state->res.body())["id"].get<snowflake>();
	// }
};

struct create_group_dm :
		request_base<create_group_dm>,
		post_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = partial_channel;
	// using return_type = snowflake;

	static std::string target() {
		return fmt::format("/users/@me/channels");
	}
	//
	// return_type overload_value() const {
	// 	return nlohmann::json::parse(state->res.body())["id"].get<snowflake>();
	// }
};

struct create_channel_invite :
		request_base<create_channel_invite>,
		post_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = invite;

	static std::string target(snowflake ch) {

		return fmt::format("/channels/{}/invites", ch);
	}
};

struct get_invite :
		request_base<get_invite>,
		get_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = invite;

	static std::string target(const std::string_view code) {

		return fmt::format("/invites/{}", code);
	}
};

struct delete_invite :
		request_base<delete_invite>,
		delete_verb {
	using request_base::request_base;
	using return_type = invite;

	static std::string target(const std::string_view code) {

		return fmt::format("/invites/{}", code);
	}
};

struct get_guild_integrations :
		request_base<get_guild_integrations>,
		get_verb {
	using request_base::request_base;
	using return_type = std::vector<guild_integration>;

	static std::string target(snowflake guild) {

		return fmt::format("/guilds/{}/integrations", guild);
	}

};

struct create_guild_integration :
		request_base<create_guild_integration>,
		post_verb,
		json_content_type {

	using request_base::request_base;
	using return_type = void;

	static std::string target(snowflake g) {

		return fmt::format("/guilds/{}/integrations", g);
	}


};

struct get_message :
		request_base<get_message>,
		get_verb {

	using request_base::request_base;
	using return_type = partial_message;

	static std::string target(snowflake ch, snowflake msg_id) {

		return fmt::format("/channels/{}/messages/{}", ch, msg_id);
	}
};


struct get_webhook :
		request_base<get_webhook>,
		get_verb {

	using request_base::request_base;
	using return_type = webhook;

	static std::string target(snowflake id) {

		return fmt::format("/webhooks/{}", id);
	}

	static std::string target(snowflake id, std::string_view token) {

		return fmt::format("/webhooks/{}/{}", id, token);
	}
};

struct get_channel_webhooks :
		request_base<get_channel_webhooks>,
		get_verb {

	using request_base::request_base;
	using return_type = std::vector<webhook>;

	static std::string target(snowflake channel) {

		return fmt::format("/channels/{}/webhooks", channel);
	}

};

struct get_guild_webhooks :
		request_base<get_guild_webhooks>,
		get_verb {

	using request_base::request_base;
	using return_type = std::vector<webhook>;

	static std::string target(snowflake guild) {

		return fmt::format("/channels/{}/webhooks", guild);
	}

};

struct create_webhook :
		request_base<create_webhook>,
		post_verb,
		json_content_type {

	using request_base::request_base;
	using return_type = webhook;

	static std::string target(snowflake ch) {

		return fmt::format("/channels/{}/webhooks", ch);
	}


};

struct execute_webhook :
		request_base<execute_webhook>,
		post_verb,
		json_content_type {

	using request_base::request_base;
	using return_type = void;

	static std::string target(const webhook& wh) {

		return fmt::format("/webhooks/{}/{}", wh.id, wh.token.value());
	}

	static std::string target(snowflake s, std::string_view token) {

		return fmt::format("/webhooks/{}/{}", s, token);
	}

};

struct modify_webhook :
		request_base<modify_webhook>,
		patch_verb,
		json_content_type {

	using request_base::request_base;
	using return_type = webhook;

	static std::string target(const webhook& wh) {

		return fmt::format("/webhooks/{}", wh.id);
	}

	static std::string target(snowflake id, std::string_view token) {

		return fmt::format("/webhooks/{}/{}", id, token);
	}

};

struct get_audit_log :
		request_base<get_audit_log>,
		get_verb {

	using request_base::request_base;
	using return_type = audit_log;

	static std::string target(snowflake g) {

		return fmt::format("/guilds/{}/audit-logs", g);
	}
};

struct get_guild_channels :
		request_base<get_guild_channels>,
		get_verb {

	using request_base::request_base;
	using return_type = std::vector<partial_channel>;

	static std::string target(snowflake g) {

		return fmt::format("/guilds/{}/channels", g);
	}
};
	
//
template<typename T, typename = void>
struct has_content_type :std::false_type {};

template<typename T>
struct has_content_type<T, std::void_t<decltype(std::declval<T>().content_type)>> :std::true_type {};

template<typename T>
static constexpr bool has_content_type_v = has_content_type<T>::value;
}


// clang-format on
