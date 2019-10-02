#pragma once
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>
#include "guild.h"
#include "ref_count_ptr.h"
#include "voice_state.h"
#include <fmt/format.h>
#include "invite.h"
#include <boost/asio.hpp>
#include <range/v3/all.hpp>

using namespace std::string_literals;
using namespace fmt::literals;

namespace rq {
	struct bad_request :std::exception {
		using exception::exception;
	};

	struct unauthorized :std::exception {
		using exception::exception;
	};

	struct forbidden :std::exception {
		using exception::exception;
	};

	struct not_found : std::exception {
		using exception::exception;
	};

	struct method_not_allowed :std::exception {
		using exception::exception;
	};

	struct gateway_unavailable :std::exception {
		using exception::exception;

		const char* what()const final {
			return "gateway unavailible try again";
		}
	};

	struct server_error :std::exception {
		using exception::exception;
		const char* what()const final {
			return "server error";
		}
	};

	struct shared_state:ref_counted{
		std::condition_variable cv{};
		std::mutex mut{};
		std::atomic<bool> done = false;
		boost::beast::http::response<boost::beast::http::string_body> res = {};
		std::vector<std::experimental::coroutine_handle<>> waiters{};
		std::mutex waiter_mut{};
		boost::asio::io_context::strand* strand = nullptr;

		void notify() {
			cv.notify_all();
			
			std::lock_guard lock(waiter_mut);
			auto all_waiters = std::move(waiters);

			const auto execute_on_strand = [&](auto&& f) {
				boost::asio::post(*strand, f);
			};
		
			ranges::for_each(waiters, execute_on_strand);			
		}
	};
	
	template<typename U, typename = void>
	struct has_overload_value :std::false_type {};

	template<typename U>
	struct has_overload_value<U, std::void_t<decltype(std::declval<U>().overload_value())>> :std::true_type {};

	static constexpr const char* application_json = "application/json";

	struct json_content_type{
		static constexpr const char* content_type = application_json;
	};

	struct delete_verb{
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
		static constexpr auto verb = boost::beast::http::verb::post;
	};

	template<typename reqeust>
	struct request_base:private crtp<reqeust> {
		request_base() = default;
		explicit request_base(ref_count_ptr<shared_state> t_state):state(std::move(t_state)){}

		template<typename ...Ts>
		static auto request(Ts&&... t) {
			return boost::beast::http::request<boost::beast::http::string_body>(
				reqeust::verb,
				"/api/v6" + reqeust::target(std::forward<Ts>(t)...),
				11
			);
		}

		void handle_errors() const {
			const auto status = state->res.result_int();
			if (status == 400) throw bad_request(";-;");
			if (status == 401) throw unauthorized();			
			if (status == 403) throw forbidden();
			if (status == 404) throw not_found();
			if (status == 405) throw method_not_allowed();
			if (status == 502) throw gateway_unavailable();
			if (status >= 500) throw server_error();
		}

		void wait() {
			std::unique_lock<std::mutex> lock{state->mut};
			state->cv.wait(lock, [this]()->bool {return ready(); });
		}

		bool ready()const noexcept {
			return state->done.load(std::memory_order_relaxed);
		}
		
		bool await_ready() const noexcept {
			return false;//await suspend checks instead
		}
		
		void await_suspend(std::experimental::coroutine_handle<> h) const{
			std::unique_lock lock(state->waiter_mut);
			if(ready()) {
				lock.unlock();
				h.resume();
			}else {
				state->waiters.push_back(h);
			}
		}

		decltype(auto) await_resume() const{	
			if constexpr(!std::is_void_v<typename reqeust::return_type>)
				return value();			
		}
				
		decltype(auto) value() const{
			if constexpr(has_overload_value<reqeust>::value)
				return this->self().overload_value();
			else if constexpr(!std::is_void_v<typename reqeust::return_type>) 
				return nlohmann::json::parse(state->res.body()).template get<typename reqeust::return_type>();			
		}

		decltype(auto) get() {
			wait();
			handle_errors();
			if constexpr(!std::is_void_v<typename reqeust::return_type>)
				return value();
		}
		
	protected:
		ref_count_ptr<shared_state> state;
	};

	struct get_guild :
		request_base<get_guild>,
		get_verb
	{
		using request_base::request_base;
		using return_type = partial_guild;
		
		static std::string target(snowflake id) {
			return "/guilds/{}"_format(id.val);
		}
	};

	struct send_message:
		request_base<send_message>,
		json_content_type,
		post_verb
	{
		using request_base::request_base;
		using return_type = partial_message;

		static std::string target(const partial_channel& channel) {	
			return "/channels/{}/messages"_format(channel.id().val);
		}

	};

	struct add_role:
		request_base<add_role>,
		put_verb
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const partial_guild& g, const guild_member& m, const guild_role& r) {
			return "/guilds/{}/members/{}/roles/{}"_format(g.id().val, m.id().val, r.id().val);
		}
	};

	struct remove_role :
		request_base<remove_role>,
		delete_verb
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const partial_guild& g, const guild_member& m, const guild_role& r) {
			return "/guilds/{}/members/{}/roles/{}"_format(g.id().val, m.id().val, r.id().val);
		}	
	};

	struct create_role:
		request_base<create_role>,
		post_verb,
		json_content_type
	{
		using request_base::request_base;
		using return_type = guild_role;

		static std::string target(const partial_guild& g) {
			return "/guilds/{}/roles"_format(g.id().val);
		}
	};

	struct delete_role:
		request_base<delete_role>,
		delete_verb
	{
		using request_base::request_base;
		using return_type = void;

		static auto target(const partial_guild& g, const guild_role& r){
			return "/guilds/{}/roles/{}"_format(g.id().val, r.id().val);
		}
	};

	struct modify_role:
		request_base<modify_role>,
		json_content_type,
		patch_verb
	{
		using request_base::request_base;
		using return_type = guild_role;

		static std::string target(const partial_guild& g, const guild_role& r) {
			return "/guilds/{}/roles/{}"_format(g.id().val, r.id().val);
		}
	};

	struct kick_member:
		request_base<kick_member>,
		delete_verb
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const partial_guild& g, const guild_member& member) {
			return "/guilds/{}/members/{}"_format(g.id().val,member.id().val);
		}
	};

	struct ban_member:
		request_base<ban_member>,
		json_content_type,
		put_verb
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const partial_guild& g, const guild_member& member) {
			return "/guilds/{}/bans/{}"_format(g.id().val, member.id().val);
		}
	};

	struct modify_member:
		request_base<modify_member>,
		json_content_type,
		patch_verb
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const partial_guild& g, const user& member) {
			return "/guilds/{}/members/{}"_format(g.id().val, member.id().val);
		}
	};

	struct change_my_nick:
		request_base<change_my_nick>,
		json_content_type,
		patch_verb
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const partial_guild& g) {
			return "/guilds/{}/members/@me/nick"_format(g.id().val);
		}
	};

	struct delete_message:
		request_base<delete_message>,
		delete_verb 
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const partial_message& msg) {
			return "/channels/{}/messages/{}"_format(msg.channel_id().val, msg.id().val);
		}
	};

	struct edit_message:
		request_base<edit_message>,
		patch_verb,
		json_content_type
	{
		using request_base::request_base;
		using return_type = partial_message;

		static std::string target(const partial_message& msg) {
			return "/channels/{}/messages/{}"_format(msg.channel_id().val,msg.id().val);
		}
	};

	struct get_messages:
		request_base<get_messages>,
		json_content_type,
		get_verb
	{
		using request_base::request_base;
		using return_type = std::vector<partial_message>;

		static std::string target(const partial_channel& channel) {
			return "/channels/{}/messages"_format(channel.id().val);
		}
	};

	struct unban_member:
		request_base<unban_member>,
		delete_verb 
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const partial_guild& guild, const snowflake& user_id) {
			return "/guilds/{}/bans/{}"_format(guild.id().val, user_id.val);
		}
	};

	struct create_text_channel:
		request_base<create_text_channel>,
		json_content_type,
		post_verb 
	{
		using request_base::request_base;
		using return_type = snowflake;

		static std::string target(const partial_guild& guild) {
			return "/guilds/{}/channels"_format(guild.id().val);
		}
		snowflake overload_value() const{
			return nlohmann::json::parse(state->res.body())["id"].get<snowflake>();
		}
	};

	struct create_voice_channel :
		request_base<create_voice_channel>,
		post_verb,
		json_content_type 
	{
		using request_base::request_base;
		using return_type = snowflake;

		static std::string target(const partial_guild& guild) {
			return "/guilds/{}/channels"_format(guild.id().val);
		}
		snowflake overload_value() const{
			return nlohmann::json::parse(state->res.body())["id"].get<snowflake>();
		}
	};

	struct create_channel_catagory :
		request_base<create_channel_catagory>,
		post_verb,
		json_content_type 
	{
		using request_base::request_base;
		using return_type = snowflake;

		static std::string target(const partial_guild& guild) {
			return "/guilds/{}/channels"_format(guild.id().val);
		}
	};

	struct delete_emoji:
		request_base<delete_emoji>,
		delete_verb 
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const partial_guild& guild, const emoji& e) {
			return "/guilds/{}/emojis/{}"_format(guild.id().val,e.id().val);
		}
	};

	struct modify_emoji:
		request_base<modify_emoji>,
		json_content_type,
		patch_verb 
	{
		using request_base::request_base;
		using return_type = emoji;

		static std::string target(const partial_guild& guild, const emoji& e) {
			return "/guilds/{}/emojis/{}"_format(guild.id().val,e.id().val);
		}
	};

	struct delete_message_bulk :
		request_base<delete_message_bulk>,
		post_verb,
		json_content_type 
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const partial_channel& channel) {
			return "/channels/{}/messages/bulk-delete"_format(channel.id().val);
		}
	};

	struct leave_guild:
		request_base<leave_guild>,
		delete_verb 
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const partial_guild& g) {
			return "/@me/guilds/{}"_format(g.id().val);
		}
	};

	struct get_voice_regions:
		request_base<get_voice_regions>
	{
		using request_base::request_base;
		using return_type = std::vector<voice_region>;

		static constexpr auto verb = boost::beast::http::verb::get;

		static std::string target() {
			return "/voice/regions";
		}
	};

	struct current_guilds:
		request_base<current_guilds>,
		get_verb
	{
		using request_base::request_base;
		using return_type = std::vector<partial_guild>;

		static std::string target() {
			return "/users/@me/guilds";
		}
	};

	struct add_reaction:
		request_base<add_reaction>,
		put_verb
	{
		using request_base::request_base;
		using return_type = void;
		//TODO make this work
		static std::string target(const partial_message& msg,const partial_emoji& emo) {
			return "/channel/{}/messages/{}/reactions/{}:{}/@me"_format(msg.channel_id().val, msg.id().val, emo.id().val, emo.name());
		}
	};

	struct typing_start:
		request_base<typing_start>,
		post_verb
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const partial_channel& ch) {
			return "/channel/{}/typing"_format(ch.id().val);
		}
	};

	struct delete_channel_permission:
		request_base<delete_channel_permission>,
		delete_verb
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const guild_channel& ch, const permission_overwrite& o) {
			return "/channels/{}/permissions/{}"_format(ch.id().val,o.id().val);
		}
	};

	struct modify_guild:
		request_base<modify_guild>,
		patch_verb,
		json_content_type
	{
		using request_base::request_base;
		using return_type = partial_guild;

		static std::string target(const partial_guild& g) {
			return "/guild/{}"_format(g.id().val);
		}
	};

	struct modify_channel_positions :
		request_base<modify_channel_positions>,
		patch_verb,
		json_content_type
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const partial_guild& g) {
			return "/guilds/{}/channels"_format(g.id().val);
		}
	};

	struct add_guild_member:
		request_base<add_guild_member>,
		put_verb,
		json_content_type
	{
		using request_base::request_base;
		using return_type = snowflake;

		static std::string target(const partial_guild& g,snowflake id) {
			return "/guilds/{}/members/{}"_format(g.id().val,id.val);
		}

		snowflake overload_value() const {
			return nlohmann::json::parse(state->res.body())["id"].get<snowflake>();
		}
	};

	struct delete_channel:
		request_base<delete_channel>,
		delete_verb
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const partial_channel& ch) {
			return "/channels/{}"_format(ch.id().val);
		}
	};

	struct add_pinned_msg:
		request_base<add_pinned_msg>,
		put_verb
	{
		using request_base::request_base;
		using return_type = void;
		static std::string target(const partial_channel& ch,const partial_message& msg) {
			return "/channels/{}/pins/{}"_format(ch.id().val, msg.id().val);
		}
	};

	struct remove_pinned_msg:
		request_base<remove_pinned_msg>,
		delete_verb 
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const partial_channel& ch, const partial_message& msg) {
			return "/channels/{}/pins/{}"_format(ch.id().val, msg.id().val);
		}
	};

	struct list_guild_members:
		request_base<list_guild_members>,
		get_verb,
		json_content_type
	{
		using request_base::request_base;
		using return_type = std::vector<guild_member>;

		static std::string target(const partial_guild& g) {
			return "/guilds/{}/members"_format(g.id().val);
		}

	};
	
	struct edit_channel_permissions:
		request_base<edit_channel_permissions>,
		put_verb,
		json_content_type
	{
		using request_base::request_base;
		using return_type = void;

		static std::string target(const guild_channel& c,const permission_overwrite& p) {
			return "/channels/{}/permissions/{}"_format(c.id().val, p.id().val);
		};
	};

	struct create_dm:
		request_base<create_dm>,
		post_verb,
		json_content_type
	{
		using request_base::request_base;
		using return_type = snowflake;

		static std::string target() {
			return "/users/@me/channels";
		}

		return_type overload_value() const{
			return nlohmann::json::parse(state->res.body())["id"].get<snowflake>();
		}
	};

	struct create_group_dm :
		request_base<create_group_dm>, 
		post_verb, 
		json_content_type 
	{
		using request_base::request_base;
		using return_type = snowflake;

		static std::string target() {
			return "/users/@me/channels";
		}

		return_type overload_value() const {
			return nlohmann::json::parse(state->res.body())["id"].get<snowflake>();
		}
	};

	struct create_channel_invite:
		request_base<create_channel_invite>,
		post_verb,
		json_content_type
	{
		using request_base::request_base;
		using return_type = invite;

		static std::string target(const guild_channel& ch) {
			return "/channels/{}/invites"_format(ch.id().val);
		}
	};

	struct get_invite:
		request_base<get_invite>,
		get_verb,
		json_content_type
	{
		using request_base::request_base;
		using return_type = invite;

		static std::string target(const std::string_view code) {
			return "/invites/{}"_format(code);
		}
	};

	struct delete_invite :
		request_base<delete_invite>,
		delete_verb 
	{
		using request_base::request_base;
		using return_type = invite;

		static std::string target(const std::string_view code) {
			return "/invites/{}"_format(code);
		}
	};
	/*
	struct create_group_dm:
		request_base<create_group_dm>,
		post_verb,
		json_content_type
	{
		using return_type = snowflake;
		static std::string target() {
			return "/users/@me/channels";
		}

		return_type overload_value() const {
			return nlohmann::json::parse(state->res.body())["id"].get<snowflake>();
		}
	};
	*/
	//not needed ;-;
	struct get_guild_channels :
		request_base<get_guild_channels>,
		get_verb 
	{
		using request_base::request_base;
		using return_type = std::vector<snowflake>;

		static std::string target(const partial_guild& g) {
			return "/guilds/{}/channels"_format(g.id().val);
		}
	};
	//
	template<typename T,typename = void>
	struct has_content_type:std::false_type{};

	template<typename T>
	struct has_content_type <T,std::void_t<decltype(std::declval<T>().content_type)>>:std::true_type {};
	
	template<typename T>
	static constexpr bool has_content_type_v = has_content_type<T>::value;
}



