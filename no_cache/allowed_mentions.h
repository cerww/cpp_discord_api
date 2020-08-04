#pragma once
#include "snowflake.h"
#include "guild_role.h"
#include "User.h"

//this file shuold be mostly the same as non_cached version

namespace cacheless {


	constexpr int allows_everyone_flag = 1;

	constexpr int allows_all_users_flag = 1 << 1;

	constexpr int allows_all_roles_flag = 1 << 2;

	constexpr int allows_roles_flag = 1 << 3;

	constexpr int allows_users_flag = 1 << 4;

	struct empty_allowed_mentions {};

	template<int flags = 0>
	struct allowed_mentions {

		static constexpr bool allow_everyone = bool(flags & allows_everyone_flag);

		static constexpr bool allow_all_roles = bool(flags & allows_all_roles_flag);

		static constexpr bool allow_all_users = bool(flags & allows_all_users_flag);

		static constexpr bool allow_some_users = bool(flags & allows_users_flag);

		static constexpr bool allow_some_roles = bool(flags & allows_roles_flag);

		static_assert(!((allow_all_roles&& allow_some_roles) || (allow_all_users && allow_some_users)));

		using vector_if_has_roles = std::conditional_t<allow_some_roles, std::vector<snowflake>, empty_allowed_mentions>;

		using vector_if_has_users = std::conditional_t<allow_some_roles, std::vector<snowflake>, empty_allowed_mentions>;

		allowed_mentions() = default;

		template<int other_flags>
		requires ((flags | other_flags) == flags)
			explicit allowed_mentions(allowed_mentions<other_flags> other) {
			using other_type = allowed_mentions<other_flags>;

			if constexpr (other_type::allow_some_roles && allow_some_roles) {
				roles_ids = std::move(other.roles_ids);
			}

			if constexpr (other_type::allow_some_users && allow_some_users) {
				user_ids = std::move(other.user_ids);
			}
		}


		allowed_mentions<flags | allows_everyone_flag> everyone()requires !allow_everyone{
			return allowed_mentions<flags | allows_everyone_flag>(*this);
		}


			allowed_mentions<flags | allows_all_roles_flag> all_roles()requires !allow_all_roles && !allow_some_roles{
				return allowed_mentions<flags | allows_all_roles_flag>(*this);
		}


			allowed_mentions<flags | allows_all_users_flag> all_users()requires !allow_all_users && !allow_some_users{
				return allowed_mentions<flags | allows_all_users_flag>(*this);
		}

			template<typename... T>
		allowed_mentions<flags | allows_roles_flag> add_roles(T&&... roles)
			requires !allow_all_roles && !allow_some_roles && (std::is_base_of_v<guild_role, std::remove_cvref_t<T>>&& ...)
		{
			auto ret = allowed_mentions<flags | allows_roles_flag>(*this);
			ret.roles_ids.reserve(sizeof...(T));
			((ret.roles_ids.push_back(roles.id())), ...);

			return ret;
		}

		template<typename... T>
		allowed_mentions<flags | allows_users_flag> add_users(T&&... users)

			requires !allow_all_users && !allow_some_users && ((std::is_base_of_v<user, std::remove_cvref_t<T>>) && ...) {

			auto ret = allowed_mentions<flags | allows_users_flag>(*this);
			ret.user_ids.reserve(sizeof...(T));
			((ret.user_ids.push_back(users.id())), ...);

			return ret;
		}

		template<typename... T>
		allowed_mentions<flags | allows_roles_flag> add_roles(T&&... roles)
			requires !allow_all_roles && !allow_some_roles && ((std::is_same_v<snowflake, std::remove_cvref_t<T>>) && ...) {

			auto ret = allowed_mentions<flags | allows_roles_flag>(*this);
			ret.roles_ids.reserve(sizeof...(T));
			((ret.roles_ids.push_back(roles)), ...);

			return ret;
		}

		template<typename... T>
		allowed_mentions<flags | allows_users_flag> add_users(T&&... users)
			requires !allow_all_users && !allow_some_users && ((std::is_same_v<snowflake, std::remove_cvref_t<T>>) && ...) {

			auto ret = allowed_mentions<flags | allows_users_flag>(*this);
			ret.user_ids.reserve(sizeof...(T));
			((ret.user_ids.push_back(users)), ...);

			return ret;
		}

		//already have the flag
		template<typename... T>
		allowed_mentions<flags>& add_roles(T&&... roles)
			requires !allow_all_roles && allow_some_roles && (std::is_base_of_v<guild_role, std::remove_cvref_t<T>>&& ...)\
		{

			((roles_ids.push_back(roles.id)), ...);
			return *this;
		}

		template<typename... T>
		allowed_mentions<flags>& add_roles(T&&... roles)
			requires !allow_all_roles && allow_some_roles && ((std::is_same_v<snowflake, std::remove_cvref_t<T>>) && ...)
		{
			((roles_ids.push_back(roles)), ...);
			return *this;
		}


		template<typename... T>
		allowed_mentions<flags>& add_users(T&&... users)
			requires !allow_all_users && allow_some_users && ((std::is_base_of_v<snowflake, std::remove_cvref_t<T>>) && ...) {

			((roles_ids.push_back(users)), ...);
			return *this;
		}

		template<typename... T>
		allowed_mentions<flags>& add_users(T&&... users)
			requires !allow_all_users && allow_some_users && ((std::is_same_v<user, std::remove_cvref_t<T>>) && ...) {

			((roles_ids.push_back(users.id)), ...);
			return *this;
		}

		//[[no_unique_address]]
		vector_if_has_roles roles_ids = {};
		//[[no_unique_address]]
		vector_if_has_users user_ids = {};
	};

	static_assert(std::is_base_of_v<user, user>);

	static inline const allowed_mentions<0> disable_mentions{};

	template<int flags>
	inline void to_json(nlohmann::json& json, const allowed_mentions<flags>& stuff) {
		json["parse"] = nlohmann::json::array_t();
		if constexpr (stuff.allow_everyone) {
			json["parse"].push_back("everyone");
		}
		if constexpr (stuff.allow_all_roles) {
			json["parse"].push_back("roles");
		}
		if constexpr (stuff.allow_all_users) {
			json["parse"].push_back("users");
		}
		if constexpr (stuff.allow_some_roles) {
			json["roles"] = stuff.roles_ids;
		}
		if constexpr (stuff.allow_some_users) {
			json["users"] = stuff.user_ids;
		}
	}

	//conditions:
	//allow_users == true => user_ids.size == 0
	//allow_roles == true => role_ids.size == 0
	//
	//
	//
	//





	struct allowed_mentions1 {
		bool everyone = false;
		bool allow_users = false;
		bool allow_roles = false;
		std::vector<snowflake> role_ids;
		std::vector<snowflake> user_ids;
	};

	//static const allowed_mentions1 disable_mentions{};

	inline void to_json(nlohmann::json& json, const allowed_mentions1& stuff) {
		json["parse"] = nlohmann::json::array_t();
		if (stuff.everyone) {
			json["parse"].push_back("everyone");
		}

		if (stuff.allow_users) {
			json["parse"].push_back("users");
		}
		else {
			if (!stuff.user_ids.empty()) {
				json["users"] = stuff.user_ids;
			}
		}

		if (stuff.allow_roles) {
			json["parse"].push_back("roles");
		}
		else {
			if (!stuff.role_ids.empty()) {
				json["roles"] = stuff.role_ids;
			}
		}
	}
};