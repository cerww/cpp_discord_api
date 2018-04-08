#pragma once
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>
#include "guild.h"
#include "ref_counted.h"
#include "voice_state.h"


using namespace std::string_literals;
namespace rq {
	static constexpr const char* application_json = "application/json";

	struct erroru:std::exception{
		erroru(std::string t):what(std::move(t)){};
		std::string what;
	};

	struct shared_state:ref_counted{//need better name plox
		std::condition_variable cv;
		bool done = false;
		std::mutex mut;
		boost::beast::http::response<boost::beast::http::string_body> res;
		size_t major_param_id = 0;
	};
	
	template<typename T>
	struct request_base:private crtp<T> {
		template<typename ...Ts>
		static auto request(Ts&&... t) {
			return boost::beast::http::request<boost::beast::http::string_body>(
				T::verb,
				"/api/v6" + T::target(std::forward<Ts>(t)...),
				11
			);
		}
		ref_count_ptr<shared_state> state;		

		void wait() {
			std::unique_lock<std::mutex> lock{state->mut};
			state->cv.wait(lock, [&]()->bool {return state->done; });
		}
		bool ready()const noexcept {
			return state->done;
		}

		bool await_ready() const noexcept {
			return ready();
		}

		void await_suspend(std::experimental::coroutine_handle<> h) {
			wait();
			h.resume();
		}

		decltype(auto) await_resume() {	
			if constexpr(!std::is_same_v<void, typename T::return_type>)
				return value();			
		}

		template<typename U, typename = void>
		struct has_overload_value :std::false_type {};

		template<typename U>
		struct has_overload_value<U, std::void_t<decltype(std::declval<U>().overload_value())>> :std::true_type {};

		decltype(auto) value() {
			if constexpr(has_overload_value<T>::value)
				return this->self().overload_value();
			else if constexpr(!std::is_same_v<void, typename T::return_type>) 
				return nlohmann::json::parse(state->res.body()).get<typename T::return_type>();			
		}
		decltype(auto) get() {
			wait();
			if constexpr(!std::is_same_v<void, typename T::return_type>)
				return value();
		}

		decltype(auto) to_future() {			
			return std::async(std::launch::async, [me = *this]()mutable {return me.get(); });
		}

		template<typename executor>
		decltype(auto) to_future(executor&& exec) {
			return std::async(std::forward<executor>(exec), [me = *this]() mutable{return me.get(); });
		}

		template<typename fn>
		auto then(fn&& Fun) {
			return std::async(std::launch::async, [this,_fun = std::forward<fn>(Fun)](){
				if constexpr(!std::is_same_v<typename T::return_type, void>)
					return _fun(get());
				else
					_fun();
			});
		}

		template<typename fn,typename executor>
		std::future<std::result_of_t<fn>> then(executor&& exec, fn&& Fun) {
			return std::async(std::forward<executor>(exec), [this, _fun = Fun](){
				if constexpr(!std::is_same_v<typename T::return_type, void>)
					return _fun(get());
				else
					_fun();
			});
		}
	};
	struct get_guild :request_base<get_guild> {
		using return_type = partial_guild;
		
		static constexpr auto verb = boost::beast::http::verb::get;
		static std::string target(snowflake id) {
			return "/guilds/" + std::to_string(id.val);
		}
	};
	struct send_message:request_base<send_message>{
		static constexpr const char* content_type = application_json;
		static constexpr auto verb = boost::beast::http::verb::post;
		using return_type = partial_message;

		static std::string target(const Channel& channel) {			
			return "/channels/"s + std::to_string(channel.id().val) + "/messages";			
		}

	};
	struct add_role:request_base<add_role>{
		static constexpr auto verb = boost::beast::http::verb::put;	
		using return_type = void;

		static std::string target(const Guild& g, const guild_member& m, const Role& r) {
			return "/guilds/" + std::to_string(g.id().val) + "/members/" + std::to_string(m.id().val) + "/roles/" + std::to_string(r.id().val);
		}
	};
	struct remove_role :request_base<remove_role>{
		static constexpr auto verb = boost::beast::http::verb::delete_;
		using return_type = void;

		static std::string target(const Guild& g, const guild_member& m, const Role& r) {
			return "/guilds/" + std::to_string(g.id().val) + "/members/" + std::to_string(m.id().val) + "/roles/" + std::to_string(r.id().val);
		}	
	};
	struct create_role:request_base<create_role>{
		static constexpr const char* content_type = application_json;
		static constexpr auto verb = boost::beast::http::verb::post;
		using return_type = Role;

		static std::string target(const Guild& g) {			
			return "/guilds/" + std::to_string(g.id().val) + "/roles";
		}
	};
	struct delete_role:request_base<delete_role>{
		static constexpr auto verb = boost::beast::http::verb::delete_;
		using return_type = void;

		static auto target(const Guild& g, const Role& r){
			return "/guilds/" + std::to_string(g.id().val) + "/roles/" + std::to_string(r.id().val);
		}
	};
	struct modify_role:request_base<modify_role>{
		static constexpr const char* content_type = application_json;
		static constexpr auto verb = boost::beast::http::verb::patch;
		using return_type = Role;

		static std::string target(const Guild& g, const Role& r) {
			return "/guilds/" + std::to_string(g.id().val) + "/roles/" + std::to_string(r.id().val);
		}
	};

	struct kick_member:request_base<kick_member> {
		static constexpr auto verb = boost::beast::http::verb::delete_;
		using return_type = void;

		static std::string target(const Guild& g, const guild_member& member) {
			return "/guilds/" + std::to_string(g.id().val) + "/members/" + std::to_string(member.id().val);
		}
	};

	struct ban_member:request_base<ban_member> {
		static constexpr auto verb = boost::beast::http::verb::put;
		static constexpr const char* content_type = application_json;
		using return_type = void;

		static std::string target(const Guild& g, const guild_member& member) {
			return "/guilds/" + std::to_string(g.id().val) + "/bans/" + std::to_string(member.id().val);
		}
	};

	struct modify_member:request_base<modify_member> {
		static constexpr const char* content_type = application_json;
		static constexpr auto verb = boost::beast::http::verb::patch;
		using return_type = void;

		static std::string target(const Guild& g, const guild_member& member) {
			return "/guilds/" + std::to_string(g.id().val) + "/members/" + std::to_string(member.id().val);
		}
	};
	struct change_my_nick:request_base<change_my_nick> {
		static constexpr const char* content_type = application_json;
		static constexpr auto verb = boost::beast::http::verb::patch;
		using return_type = void;

		static std::string target(const Guild& g) {
			return "/guilds/" + std::to_string(g.id().val) + "/members/@me/nick";
		}	
	};

	struct delete_message:request_base<delete_message> {
		static constexpr auto verb = boost::beast::http::verb::delete_;
		using return_type = void;

		static std::string target(const partial_message& msg) {
			return "/channels/" + std::to_string(msg.channel_id().val) + "/messages/" + std::to_string(msg.id().val);
		}
	};

	struct edit_message:request_base<edit_message> {
		static constexpr const char* content_type = application_json;
		static constexpr auto verb = boost::beast::http::verb::patch;
		using return_type = partial_message;

		static std::string target(const partial_message& msg) {
			return "/channels/" + std::to_string(msg.channel_id().val) + "/messages/" + std::to_string(msg.id().val);
		}
	};
	struct get_messages:request_base<get_messages> {
		static constexpr auto verb = boost::beast::http::verb::get;
		static constexpr const char* content_type = application_json;
		using return_type = std::vector<partial_message>;

		static std::string target(const Channel& channel) {
			return "/channels/" + std::to_string(channel.id().val) + "/messages";
		}
	};

	struct unban_member:request_base<unban_member> {
		static constexpr auto verb = boost::beast::http::verb::delete_;
		using return_type = void;

		static std::string target(const Guild& guild, const snowflake& user_id) {
			return "/guilds/" + std::to_string(guild.id().val) + "/bans/" + std::to_string(user_id.val);
		}
	};

	struct create_text_channel:request_base<create_text_channel> {
		static constexpr const char* content_type = application_json;
		static constexpr auto verb = boost::beast::http::verb::post;
		using return_type = snowflake;

		static std::string target(const Guild& guild) {
			return "/guilds/" + std::to_string(guild.id().val) + "/channels";
		}
		snowflake overload_value() {
			return nlohmann::json::parse(state->res.body())["id"].get<snowflake>();
		}
	};

	struct create_voice_channel :request_base<create_voice_channel> {
		static constexpr auto verb = boost::beast::http::verb::post;
		static constexpr const char* content_type = application_json;
		using return_type = snowflake;

		static std::string target(const Guild& guild) {
			return "/guilds/" + std::to_string(guild.id().val) + "/channels";
		}
		snowflake overload_value() {
			return nlohmann::json::parse(state->res.body())["id"].get<snowflake>();
		}
	};
	struct create_channel_catagory :request_base<create_channel_catagory> {
		static constexpr const char* content_type = application_json;
		static constexpr auto verb = boost::beast::http::verb::post;
		using return_type = snowflake;

		static std::string target(const Guild& guild) {
			return "/guilds/" + std::to_string(guild.id().val) + "/channels";
		}
	};

	struct delete_emoji:request_base<delete_emoji> {
		static constexpr auto verb = boost::beast::http::verb::delete_;
		using return_type = void;

		static std::string target(const Guild& guild, const emoji& e) {
			return "/guilds/" + std::to_string(guild.id().val) + "/emojis/" + std::to_string(e.id().val);
		}
	};

	struct modify_emoji:request_base<modify_emoji> {
		static constexpr auto verb = boost::beast::http::verb::patch;
		using return_type = emoji;

		static std::string target(const Guild& guild, const emoji& e) {
			return "/guilds/" + std::to_string(guild.id().val) + "/emojis/" + std::to_string(e.id().val);
		}
	};

	struct delete_message_bulk :request_base<delete_message_bulk> {
		static constexpr const char* content_type = application_json;

		static constexpr auto verb = boost::beast::http::verb::post;
		using return_type = void;
		static std::string target(const Channel& channel) {
			return "/channels" + std::to_string(channel.id().val) + "/messages/bulk-delete";
		}
	};

	struct leave_guild:request_base<leave_guild> {
		using return_type = void;
		static constexpr auto verb = boost::beast::http::verb::delete_;

		static std::string target(const Guild& g) {
			return "/@me/guilds/" + std::to_string(g.id().val);
		}
	};

	struct get_voice_regions:request_base<get_voice_regions>{
		using return_type = std::vector<voice_region>;
		static constexpr auto verb = boost::beast::http::verb::get;
		static std::string target() {
			return "/voice/regions";
		}
	};

	struct current_guilds:request_base<current_guilds>{
		using return_type = std::vector<partial_guild>;
		static constexpr auto verb = boost::beast::http::verb::get;
		static std::string target() {
			return "/users/@me/guilds";
		}
	};
	struct add_reaction:request_base<add_reaction>{
		using return_type = void;
		static constexpr auto verb = boost::beast::http::verb::put;
		static std::string target(const partial_message& msg,const partial_emoji& emo) {
			return "/channel/" + std::to_string(msg.channel_id().val) + "/messages/" + std::to_string(msg.id().val) + "/reactions/" + std::to_string(emo.id().val) + ":" + emo.name() + "/@me";
		}
	};

	struct typing_start:request_base<typing_start>{
		using return_type = void;
		static constexpr auto verb = boost::beast::http::verb::post;
		static std::string target(const Channel& ch) {
			return "/channel/" + std::to_string(ch.id().val) + "/typing";
		}
	};

	struct delete_channel_permission:request_base<delete_channel_permission>{
		using return_type = void;
		static constexpr auto verb = boost::beast::http::verb::delete_;
		static std::string target(const guild_channel& ch, const permission_overwrite& o) {
			return "/channels/"+ std::to_string(ch.id().val) + "/permissions/" + std::to_string(o.id.val);
		}
	};

	struct modify_guild:request_base<modify_guild>{
		using return_type = partial_guild;
		static constexpr auto verb = boost::beast::http::verb::patch;
		static constexpr const char* content_type = application_json;
		static std::string target(const Guild& g) {
			return "/guild/" + std::to_string(g.id().val);
		}
	};

	struct modify_channel_positions :request_base<modify_channel_positions>{
		using return_type = void;
		static constexpr auto verb = boost::beast::http::verb::patch;
		static constexpr const char* content_type = application_json;
		static std::string target(const Guild& g) {
			return "/guilds/" + std::to_string(g.id().val) + "/channels";
		}
	};

	struct add_guild_member:request_base<add_guild_member>{
		using return_type = snowflake;
		static constexpr auto verb = boost::beast::http::verb::put;
		static constexpr const char* content_type = application_json;
		static std::string target(const Guild& g,snowflake id) {
			return "/guilds/" + std::to_string(g.id().val) + "/members/" + std::to_string(id.val);
		}
		snowflake overload_value() {
			return nlohmann::json::parse(state->res.body())["id"].get<snowflake>();
		}
	};

	struct delete_channel:request_base<delete_channel>{
		using return_type = void;
		static constexpr auto verb = boost::beast::http::verb::delete_;
		static std::string target(const Channel& ch) {
			return "/channels/" + std::to_string(ch.id().val);
		}
	};

	struct add_pinned_msg:request_base<add_pinned_msg>{
		using return_type = void;
		static constexpr auto verb = boost::beast::http::verb::put;
		static std::string target(const Channel& ch,const partial_message& msg) {
			return "/channels/" + std::to_string(ch.id().val) + "/pins/" + std::to_string(msg.id().val);
		}
	};

	struct remove_pinned_msg:request_base<remove_pinned_msg> {
		using return_type = void;
		static constexpr auto verb = boost::beast::http::verb::delete_;
		static std::string target(const Channel& ch, const partial_message& msg) {
			return "/channels/" + std::to_string(ch.id().val) + "/pins/" + std::to_string(msg.id().val);
		}
	};

	template<typename T,typename = void>
	struct has_content_type:std::false_type{};

	template<typename T>
	struct has_content_type <T,std::void_t<decltype(T::content_type)>>:std::true_type {};

	template<typename T>
	static constexpr bool has_content_type_v = has_content_type<T>::value;
}



