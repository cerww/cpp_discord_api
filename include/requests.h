#pragma once
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>
#include "guild.h"
#include "../common/ref_count_ptr.h"
#include "voice_state.h"
#include <fmt/format.h>
#include "invite.h"
#include <boost/asio.hpp>
#include <range/v3/all.hpp>
#include "guild_integration.h"
#include "webhook.h"
#include <ranges>
#include <algorithm>
#include "audit_log.h"
#include <fmt/compile.h>
#include <algorithm>
#include "dm_channel.h"
#include "partial_message.h"
#include "text_channel.h"
#include "http_connection.h"


// clang-format off
namespace rq {
using namespace std::string_literals;
using namespace fmt::literals;

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

	const char* what() const override {
		return "gateway unavailible try again";
	}
};

struct server_error final :std::exception {
	using exception::exception;

	const char* what() const override {
		return "server error";
	}
};

struct shared_state2 {	
	std::optional<std::exception_ptr> exception;//rename?
	boost::asio::io_context::strand* strand = nullptr;
	boost::beast::http::request<boost::beast::http::string_body> req;
	boost::beast::http::response<boost::beast::http::string_body> res;
	http_connection2* http_connection = nullptr;
};

static constexpr int asuodjhasdasd = sizeof(shared_state2);

// struct shared_state :ref_counted {
// 	//rename some of these?
// 	std::atomic<bool> done = false;
// 	boost::asio::io_context::strand* strand = nullptr;
// 	std::vector<std::experimental::coroutine_handle<>> waiters{};
// 	std::mutex ready_mut{};
// 	std::mutex waiter_mut{};
// 	std::condition_variable ready_cv{};
// 	boost::beast::http::response<boost::beast::http::string_body> res = {};
// 	std::optional<std::exception_ptr> exception;//rename?
// 	std::atomic<bool> is_canceled = false;
//
// 	void finish() {
// 		done = true;
// 		ready_cv.notify_all();
//
// 		auto all_waiters = [&] {
// 			std::lock_guard lock(waiter_mut);
// 			return std::move(waiters);
// 		}();
//
// 		const auto execute_on_strand = [&](auto&& f) {
// 			boost::asio::post(*strand, f);
// 		};
//
// 		std::ranges::for_each(all_waiters, execute_on_strand);
//
// 	}
// };



constexpr static int ajkdhas = sizeof(std::condition_variable);

template<typename U, typename = void>
struct has_overload_value :std::false_type {};

template<typename U>
struct has_overload_value<U, std::void_t<decltype(std::declval<U>().overload_value(
	std::declval<boost::beast::http::response<boost::beast::http::string_body>>()))
>> :std::true_type {};

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

template<typename request_t>
struct request_base :private crtp<request_t> {
	request_base() = default;

	explicit request_base(shared_state2 t_state) :
		state(std::move(t_state)) {}

	template<typename T>
	explicit request_base(with_exception<T> e, boost::asio::io_context::strand* strand) {
		state.strand = strand;		
		state.exception = std::make_exception_ptr(e.exception);
		
	}

	template<typename ...Ts>
	static auto request(Ts&&... t) {
		return boost::beast::http::request<boost::beast::http::string_body>(
			request_t::verb,
			"/api/v8" + request_t::target(std::forward<Ts>(t)...),
			11
			);
	}

	void handle_errors() const {
		if (state.exception.has_value()) {
			std::rethrow_exception(state.exception.value());
		}

		const auto status = state.res.result_int();
		if (status == 400) {
			throw bad_request(";-;");
		}else if (status == 401) {
			throw unauthorized();
		}else if (status == 403) {
			throw forbidden();
		}else if (status == 404) {
			throw not_found();
		}else if (status == 405) {
			throw method_not_allowed();
		}else if (status == 502) {
			throw gateway_unavailable();
		}else if (status >= 500) {
			throw server_error();
		}
	}
	
	bool await_ready() const noexcept {
		return state.exception.has_value();
	}

	void await_suspend(std::experimental::coroutine_handle<> h) {
		discord_request r;
		r.req = std::move(state.req);
		r.on_finish = [this,h](auto res) {
			state.res = std::move(res);
			boost::asio::post(*state.strand, h);
		};
		state.http_connection->send(std::move(r));
	}

	decltype(auto) value() const {
		// ReSharper disable CppRedundantTemplateKeyword
		handle_errors();
		if constexpr (has_overload_value<request_t>::value)
			return this->self().overload_value(state.res);
		else if constexpr (!std::is_void_v<typename request_t::return_type>) {
			return nlohmann::json::parse(state.res.body()).template get<typename request_t::return_type>();
		}		
		// ReSharper restore CppRedundantTemplateKeyword		
	}

	decltype(auto) await_resume() const {
		if constexpr (!std::is_void_v<typename request_t::return_type>)
			return value();
	}	

	void execute_and_ignore() {
		if (!state.exception.has_value()) {
			discord_request r;
			r.req = std::move(state.req);
			state.http_connection->send(std::move(r));
		}
	}

	auto ignoring_result() {
		struct awaiter {
			request_base r;

			bool await_ready() {
				return r.state.exception.has_value();
			}

			void await_suspend(std::experimental::coroutine_handle<> h) {
				discord_request ret;
				ret.req = std::move(r.state.req);
				ret.on_finish = [this,h](auto res) {
					r.state.res = std::move(res);
					boost::asio::post(*r.state.strand, h);
				};
				r.state.http_connection->send(std::move(ret));
			}

			void await_resume() {
				r.handle_errors();
			}
			
		};		
		return awaiter{ std::move(*this) };
	}

	auto execute_await_later() {

		struct stuffs {
			explicit stuffs(request_t r):rq(std::move(r)){}
			
			request_t rq;
			bool done = false;
			std::experimental::coroutine_handle<> waiter;
			std::mutex mut;
		};
		
		struct awaiter {	

			bool await_ready() {
				return shared_state->execption.has_value();
			}

			bool await_suspend(std::experimental::coroutine_handle<> h) {
				std::lock_guard lock(shared_state->mut);
				if(shared_state->done) {					
					return true;
				}else {
					shared_state->waiter = h;
					return false;
				}
			}

			decltype(auto) await_resume() {
				return shared_state->rq.await_ready();
			};

			
			std::shared_ptr<stuffs> shared_state;
		};
		
		std::shared_ptr<stuffs> shared_state = std::make_shared<stuffs>(std::move(this->self()));		
		
		if (!state.exception.has_value()) {
			discord_request r;
			r.req = std::move(shared_state.rq.state.req);
			r.on_finish = [=,shared_state](auto res) {
				{
					std::lock_guard lock(shared_state->mut);
					shared_state->done = true;
					shared_state->rq.state.res = std::move(res);
				}
				//shared_state->done == true => the branch that modifies shared_state->waiter won't be executed
				if (shared_state->waiter) {
					boost::asio::post(*shared_state->rq.state.strand, shared_state->waiter);
				}
			};			
			state.http_connection->send(std::move(r));
		}
		return awaiter{ std::move(shared_state) };
	}
	
protected:
	shared_state2 state;
};

// template<typename reqeust>
// struct request_base2 :private crtp<reqeust> {
// 	request_base2() = default;
//
// 	explicit request_base2(ref_count_ptr<shared_state> t_state):
// 		state(std::move(t_state)) {}
//
// 	template<typename T>
// 	explicit request_base2(with_exception<T> e, boost::asio::io_context::strand* strand):
// 		request_base2(make_ref_count_ptr<shared_state>()) {
// 		state->strand = strand;
// 		state->done = true;
// 		state->exception = std::make_exception_ptr(e.exception);
// 	}
//
// 	template<typename ...Ts>
// 	static auto request(Ts&&... t) {
// 		return boost::beast::http::request<boost::beast::http::string_body>(
// 			reqeust::verb,
// 			"/api/v6" + reqeust::target(std::forward<Ts>(t)...),
// 			11
// 		);
// 	}
//
// 	void handle_errors() const {
// 		if (state->exception.has_value()) {
// 			std::rethrow_exception(state->exception.value());
// 		}else if(state->is_canceled) {
// 			throw std::runtime_error("canceled");
// 		}
// 		const auto status = state->res.result_int();
// 		if (status == 400)
// 			throw bad_request(";-;");
// 		if (status == 401)
// 			throw unauthorized();
// 		if (status == 403)
// 			throw forbidden();
// 		if (status == 404)
// 			throw not_found();
// 		if (status == 405)
// 			throw method_not_allowed();
// 		if (status == 502)
// 			throw gateway_unavailable();
// 		if (status >= 500)
// 			throw server_error();
// 	}
//
// 	void wait() {
// 		std::unique_lock<std::mutex> lock{state->ready_mut};
// 		state->ready_cv.wait(lock, [this]()->bool { return ready(); });
// 	}
//
// 	bool ready() const noexcept {
// 		return state->done.load(std::memory_order_relaxed);
// 	}
//
// 	// ReSharper disable CppMemberFunctionMayBeStatic
// 	bool await_ready() const noexcept {
// 		// ReSharper restore CppMemberFunctionMayBeStatic
// 		return false;//await suspend checks instead
// 	}
//
// 	void await_suspend(std::experimental::coroutine_handle<> h) const {
// 		std::unique_lock lock(state->waiter_mut);
// 		if (ready()) {
// 			lock.unlock();
// 			h.resume();
// 		} else {
// 			state->waiters.push_back(h);
// 		}
// 	}
//
// 	decltype(auto) await_resume() const {
// 		if constexpr (!std::is_void_v<typename reqeust::return_type>)
// 			return value();
// 	}
//
// 	decltype(auto) value() const {
// 		handle_errors();
// 		if constexpr (has_overload_value<reqeust>::value)
// 			return this->self().overload_value(state->res);
// 		else if constexpr (!std::is_void_v<typename reqeust::return_type>)
// 			return nlohmann::json::parse(state->res.body()).template get<typename reqeust::return_type>();
// 	}
// 		
// 	//remove along with wait?
// 	decltype(auto) get() {
// 		wait();
// 		handle_errors();
// 		if constexpr (!std::is_void_v<typename reqeust::return_type>) {
// 			return value();
// 		}
// 	}
// 		
// 	//returns awaitable and ignores the value returned from discord's api
// 	auto async_wait() {
// 		struct r {
// 			request_base* me = nullptr;
//
// 			bool await_ready() {
// 				return false;
// 			}
//
// 			decltype(auto) await_suspend(std::experimental::coroutine_handle<> h) {
// 				me->await_suspend(h);
// 			}
//
// 			// ReSharper disable once CppMemberFunctionMayBeStatic
// 			decltype(auto) await_resume() {
// 				//me->handle_errors();
// 				//return me->await_resume();
// 			}
//
// 		};
// 		return r{this};
// 	}
//
// 	void cancel() const{
// 		//don't set to canceled if already finished
// 		if (!state->done) {
// 			state->is_canceled = true;
// 		}
// 	}
//
// protected:
// 	ref_count_ptr<shared_state> state;
// };

struct get_guild :
		request_base<get_guild>,
		get_verb {
	using request_base::request_base;
	using return_type = partial_guild;

	static std::string target(snowflake id) {
		return "/guilds/{}"_format(id);
	}
};

struct send_message :
		request_base<send_message>,
		json_content_type,
		post_verb {
	using request_base::request_base;
	using return_type = partial_message;

	static std::string target(const partial_channel& channel) {		
		return fmt::format(FMT_STRING("/channels/{}/messages"),channel.id());
	}

	static std::string target(const partial_message& msg) {		
		return fmt::format(FMT_STRING("/channels/{}/messages"), msg.channel_id());
	}
};

struct send_message_with_file :
		request_base<send_message_with_file>,
		json_content_type,
		post_verb {
	using request_base::request_base;
	using return_type = partial_message;

	static std::string target(const partial_channel& channel) {
		return "/channels/{}/messages"_format(channel.id());
	}
};

struct add_role :
		request_base<add_role>,
		put_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_guild& g, const partial_guild_member& m, const guild_role& r) {
		return "/guilds/{}/members/{}/roles/{}"_format(g.id(), m.id(), r.id());
	}
};

struct remove_role :
		request_base<remove_role>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_guild& g, const partial_guild_member& m, const guild_role& r) {
		return "/guilds/{}/members/{}/roles/{}"_format(g.id(), m.id(), r.id());
	}
};

struct create_role :
		request_base<create_role>,
		post_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = guild_role;

	static std::string target(const partial_guild& g) {
		return "/guilds/{}/roles"_format(g.id());
	}
};

struct delete_role :
		request_base<delete_role>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static auto target(const partial_guild& g, const guild_role& r) {
		return "/guilds/{}/roles/{}"_format(g.id(), r.id());
	}
};

struct modify_role :
		request_base<modify_role>,
		json_content_type,
		patch_verb {
	using request_base::request_base;
	using return_type = guild_role;

	static std::string target(const partial_guild& g, const guild_role& r) {
		return "/guilds/{}/roles/{}"_format(g.id(), r.id());
	}
};

struct kick_member :
		request_base<kick_member>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_guild& g, const partial_guild_member& member) {
		return "/guilds/{}/members/{}"_format(g.id(), member.id());
	}
};

struct ban_member :
		request_base<ban_member>,
		json_content_type,
		put_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_guild& g, const partial_guild_member& member) {
		return "/guilds/{}/bans/{}"_format(g.id(), member.id());
	}
};

struct modify_member :
		request_base<modify_member>,
		json_content_type,
		patch_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_guild& g, const user& member) {
		return "/guilds/{}/members/{}"_format(g.id(), member.id());
	}
};

struct change_my_nick :
		request_base<change_my_nick>,
		json_content_type,
		patch_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_guild& g) {
		return "/guilds/{}/members/@me/nick"_format(g.id());
	}
};

struct delete_message :
		request_base<delete_message>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_message& msg) {
		return "/channels/{}/messages/{}"_format(msg.channel_id(), msg.id());
	}
};

struct edit_message :
		request_base<edit_message>,
		patch_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = partial_message;

	static std::string target(const partial_message& msg) {
		return "/channels/{}/messages/{}"_format(msg.channel_id(), msg.id());
	}
};

struct get_messages :
		request_base<get_messages>,
		json_content_type,
		get_verb {
	using request_base::request_base;
	using return_type = std::vector<partial_message>;

	static std::string target(const partial_channel& channel) {
		return "/channels/{}/messages"_format(channel.id());
	}
};

struct unban_member :
		request_base<unban_member>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_guild& guild, snowflake user_id) {
		return "/guilds/{}/bans/{}"_format(guild.id(), user_id);
	}
};

struct create_text_channel :
		request_base<create_text_channel>,
		json_content_type,
		post_verb {
	using request_base::request_base;
	//using return_type = snowflake;
	using return_type = partial_guild_channel;

	static std::string target(const partial_guild& guild) {
		return "/guilds/{}/channels"_format(guild.id());
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
	using return_type = partial_guild_channel;

	static std::string target(const partial_guild& guild) {
		return "/guilds/{}/channels"_format(guild.id());
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

	static std::string target(const partial_guild& guild) {
		return "/guilds/{}/channels"_format(guild.id());
	}
};

struct delete_emoji :
		request_base<delete_emoji>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_guild& guild, const partial_emoji& e) {
		return "/guilds/{}/emojis/{}"_format(guild.id(), e.id());
	}
};

struct modify_emoji :
		request_base<modify_emoji>,
		json_content_type,
		patch_verb {
	using request_base::request_base;
	using return_type = emoji;

	static std::string target(const partial_guild& guild, const partial_emoji& e) {
		return "/guilds/{}/emojis/{}"_format(guild.id(), e.id());
	}
};

struct delete_message_bulk :
		request_base<delete_message_bulk>,
		post_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_channel& channel) {
		return "/channels/{}/messages/bulk-delete"_format(channel.id());
	}
};

struct leave_guild :
		request_base<leave_guild>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_guild& g) {
		return "/@me/guilds/{}"_format(g.id());
	}
};

struct get_voice_regions :
		request_base<get_voice_regions> {
	using request_base::request_base;
	using return_type = std::vector<voice_region>;

	static constexpr auto verb = boost::beast::http::verb::get;

	static std::string target() {
		return "/voice/regions";
	}
};

struct current_guilds :
		request_base<current_guilds>,
		get_verb {
	using request_base::request_base;
	using return_type = std::vector<partial_guild>;

	static std::string target() {
		return "/users/@me/guilds";
	}
};

struct add_reaction :
		request_base<add_reaction>,
		put_verb {
	using request_base::request_base;
	using return_type = void;


	static std::string target(const partial_message& msg, const partial_emoji& emo) {
		return "/channels/{}/messages/{}/reactions/{}/@me"_format(msg.channel_id(), msg.id(), emo.to_reaction_string());
	}
};

struct delete_own_reaction :
		request_base<delete_own_reaction>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_message& msg, const partial_emoji& emoji) {
		return "/channels/{}/messages/{}/reactions/{}/@me"_format(msg.channel_id(), msg.id(), emoji.to_reaction_string());
	}

	static std::string target(const partial_message& msg, const reaction& reaction) {
		return target(msg, reaction.emoji());
	}
};

struct delete_user_reaction :
		request_base<delete_user_reaction>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_message& msg, const partial_emoji& emoji, const user& member) {
		return "/channels/{}/messages/{}/reactions/{}/{}"_format(msg.channel_id(), msg.id(), emoji.to_reaction_string(), member.id());
	}

	static std::string target(const partial_message& msg, const reaction& react, const user& member) {
		return target(msg, react.emoji(), member);
	}
};

struct get_reactions :
		request_base<get_reactions>,
		get_verb {

	using request_base::request_base;
	using return_type = std::vector<user>;

	static std::string target(const partial_message& msg, const partial_emoji& emoji) {
		return "/channels/{}/messages/{}/reactions/{}"_format(msg.channel_id(), msg.id(), emoji.to_reaction_string());
	}

	static std::string target(const partial_message& msg, const reaction& emoji) {
		return "/channels/{}/messages/{}/reactions/{}"_format(msg.channel_id(), msg.id(), emoji.emoji().to_reaction_string());
	}
};

struct delete_all_reactions :
		request_base<delete_all_reactions>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_message& msg) {
		return "/channels/{}/messages/{}/reactions"_format(msg.channel_id(), msg.id());
	}
};

struct delete_all_reactions_emoji :
		request_base<delete_all_reactions_emoji>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_message& msg, const partial_emoji& emoji) {
		return "/channels/{}/messages/{}/reactions/{}"_format(msg.channel_id(), msg.id(), emoji.to_reaction_string());
	}

	static std::string target(const partial_message& msg, const reaction& reaction) {
		return target(msg, reaction.emoji());
	}
};

struct typing_start :
		request_base<typing_start>,
		post_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_channel& ch) {
		return "/channels/{}/typing"_format(ch.id());
	}
};

struct delete_channel_permission :
		request_base<delete_channel_permission>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_guild_channel& ch, const permission_overwrite& o) {
		return "/channels/{}/permissions/{}"_format(ch.id(), o.id());
	}
};

struct modify_guild :
		request_base<modify_guild>,
		patch_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = partial_guild;

	static std::string target(const partial_guild& g) {
		return "/guild/{}"_format(g.id());
	}
};

struct modify_channel_positions :
		request_base<modify_channel_positions>,
		patch_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_guild& g) {
		return "/guilds/{}/channels"_format(g.id());
	}
};

struct add_guild_member :
		request_base<add_guild_member>,
		put_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = snowflake;

	static std::string target(const partial_guild& g, snowflake id) {
		return "/guilds/{}/members/{}"_format(g.id(), id);
	}

	snowflake overload_value(const boost::beast::http::response<boost::beast::http::string_body>&) const {
		return nlohmann::json::parse(state.res.body())["id"].get<snowflake>();
	}
};

struct delete_channel :
		request_base<delete_channel>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_channel& ch) {
		return "/channels/{}"_format(ch.id());
	}
};

struct add_pinned_msg :
		request_base<add_pinned_msg>,
		put_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_channel& ch, const partial_message& msg) {
		return "/channels/{}/pins/{}"_format(ch.id(), msg.id());
	}
};

struct remove_pinned_msg :
		request_base<remove_pinned_msg>,
		delete_verb {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_channel& ch, const partial_message& msg) {
		return "/channels/{}/pins/{}"_format(ch.id(), msg.id());
	}
};

struct list_guild_members :
		request_base<list_guild_members>,
		get_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = std::vector<guild_member>;

	static std::string target(const partial_guild& g) {
		return "/guilds/{}/members"_format(g.id());
	}

};

struct edit_channel_permissions :
		request_base<edit_channel_permissions>,
		put_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_guild_channel& c, const permission_overwrite& p) {
		return "/channels/{}/permissions/{}"_format(c.id(), p.id());
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
		return "/users/@me/channels";
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
		return "/users/@me/channels";
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

	static std::string target(const partial_guild_channel& ch) {
		return "/channels/{}/invites"_format(ch.id());
	}
};

struct get_invite :
		request_base<get_invite>,
		get_verb,
		json_content_type {
	using request_base::request_base;
	using return_type = invite;

	static std::string target(const std::string_view code) {
		return "/invites/{}"_format(code);
	}
};

struct delete_invite :
		request_base<delete_invite>,
		delete_verb {
	using request_base::request_base;
	using return_type = invite;

	static std::string target(const std::string_view code) {
		return "/invites/{}"_format(code);
	}
};

struct get_guild_integrations :
		request_base<get_guild_integrations>,
		get_verb {
	using request_base::request_base;
	using return_type = std::vector<guild_integration>;

	static std::string target(const partial_guild& guild) {
		return "/guilds/{}/integrations"_format(guild.id());
	}

};

struct create_guild_integration :
		request_base<create_guild_integration>,
		post_verb,
		json_content_type {

	using request_base::request_base;
	using return_type = void;

	static std::string target(const partial_guild& g) {
		return "/guilds/{}/integrations"_format(g.id());
	}


};

struct get_message :
		request_base<get_message>,
		get_verb {

	using request_base::request_base;
	using return_type = partial_message;

	static std::string target(const partial_channel& ch, snowflake msg_id) {
		return "/channels/{}/messages/{}"_format(ch.id(), msg_id);
	}
};

struct get_webhook :
		request_base<get_webhook>,
		get_verb {

	using request_base::request_base;
	using return_type = webhook;

	static std::string target(snowflake id) {
		return "/webhooks/{}"_format(id);
	}

	static std::string target(snowflake id, std::string_view token) {
		return "/webhooks/{}/{}"_format(id, token);
	}
};

struct get_channel_webhooks :
		request_base<get_channel_webhooks>,
		get_verb {

	using request_base::request_base;
	using return_type = std::vector<webhook>;

	static std::string target(const partial_channel& channel) {
		return "/channels/{}/webhooks"_format(channel.id());
	}
};

struct get_guild_webhooks :
		request_base<get_guild_webhooks>,
		get_verb {

	using request_base::request_base;
	using return_type = std::vector<webhook>;

	static std::string target(const partial_guild& guild) {
		return "/channels/{}/webhooks"_format(guild.id());
	}

};

struct create_webhook :
		request_base<create_webhook>,
		post_verb,
		json_content_type {

	using request_base::request_base;
	using return_type = webhook;

	static std::string target(const partial_channel& ch) {
		return "/channels/{}/webhooks"_format(ch.id());
	}
};

struct execute_webhook :
		request_base<execute_webhook>,
		post_verb,
		json_content_type {

	using request_base::request_base;
	using return_type = void;

	static std::string target(const webhook& wh) {
		return "/webhooks/{}/{}"_format(wh.id(), wh.token().value());
	}

	static std::string target(snowflake s, std::string_view token) {
		return "/webhooks/{}/{}"_format(s, token);
	}

};

struct modify_webhook :
		request_base<modify_webhook>,
		patch_verb,
		json_content_type {

	using request_base::request_base;
	using return_type = webhook;

	static std::string target(const webhook& wh) {
		return "/webhooks/{}"_format(wh.id());
	}

	static std::string target(snowflake id, std::string_view token) {
		return "/webhooks/{}/{}"_format(id, token);
	}
};

struct get_audit_log :
		request_base<get_audit_log>,
		get_verb {

	using request_base::request_base;
	using return_type = audit_log;

	static std::string target(const partial_guild& g) {
		return "/guilds/{}/audit-logs"_format(g.id());
	}
};

//not needed ;-;
struct get_guild_channels :
		request_base<get_guild_channels>,
		get_verb {
	using request_base::request_base;
	using return_type = std::vector<snowflake>;

	static std::string target(const partial_guild& g) {
		return "/guilds/{}/channels"_format(g.id());
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
