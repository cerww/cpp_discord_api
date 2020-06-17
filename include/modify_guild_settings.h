#pragma once
#include <string>
#include "snowflake.h"


template<typename T>
struct modify_guild_setting :crtp<T> { };

struct guild_settings {

	struct name {
		static constexpr std::string_view vname = "name";
		std::string n;
	};

	struct voice_region {
		std::string n;
		static constexpr std::string_view vname = "voice_region";
	};

	struct verification_level {
		int n = 0;
		static constexpr std::string_view vname = "verification_level";
	};

	struct default_message_notifications {
		int n = 0;
		static constexpr std::string_view vname = "default_message_notifications";
	};

	struct explicit_content_filter {
		int n = 0;
		static constexpr std::string_view vname = "explicit_content_filter";
	};

	struct afk_channel_id {
		snowflake n;
		static constexpr std::string_view vname = "afk_channel_id";
	};

	struct afk_timeout {
		int n = 0;
		static constexpr std::string_view vname = "afk_timeout";
	};

	struct icon {
		std::string n;
		static constexpr std::string_view vname = "icon";
	};

	struct owner_id {
		snowflake n;
		static constexpr std::string_view vname = "owner_id";
	};

	struct splash {
		std::string n;
		static constexpr std::string_view vname = "splash";
	};

	struct banner {
		std::string n;
		static constexpr std::string_view vname = "banner";
	};

	struct system_channel_id {
		snowflake n;
		static constexpr std::string_view vname = "system_channel_id";
	};
};


template<typename ... Ts>
struct modify_guild_settings {
	auto name(std::string n) {
		return modify_guild_settings<Ts..., guild_settings::name>{
			std::tuple_cat(std::move(stuff),
						   std::tuple(guild_settings::name{std::move(n)}))
		};
	}

	auto voice_region(std::string n) {
		return modify_guild_settings<Ts..., guild_settings::voice_region>{
			std::tuple_cat(std::move(stuff),
						   std::tuple(guild_settings::voice_region{std::move(n)}))
		};
	}

	auto voice_region(::voice_region& n) {
		return modify_guild_settings<Ts..., guild_settings::voice_region>{
			std::tuple_cat(std::move(stuff),
				std::tuple(guild_settings::voice_region{ std::string(n.id()) }))
		};
	}

	auto verification_level(int n) {
		return modify_guild_settings<Ts..., guild_settings::verification_level>{
			std::tuple_cat(std::move(stuff),
						   std::tuple(guild_settings::verification_level{std::move(n)}))
		};
	}

	auto default_message_notifications(int n) {
		return modify_guild_settings<Ts..., guild_settings::default_message_notifications>{
			std::tuple_cat(std::move(stuff),
						   std::tuple(guild_settings::default_message_notifications{std::move(n)}))
		};
	}

	auto explicit_content_filter(int n) {
		return modify_guild_settings<Ts..., guild_settings::explicit_content_filter>{
			std::tuple_cat(std::move(stuff),
						   std::tuple(guild_settings::explicit_content_filter{std::move(n)}))
		};
	}

	auto afk_channel_id(snowflake n) {
		return modify_guild_settings<Ts..., guild_settings::afk_channel_id>{
			std::tuple_cat(std::move(stuff),
						   std::tuple(guild_settings::afk_channel_id{std::move(n)}))
		};
	}

	auto afk_timeout(int n) {
		return modify_guild_settings<Ts..., guild_settings::afk_timeout>{
			std::tuple_cat(std::move(stuff),
						   std::tuple(guild_settings::afk_timeout{std::move(n)}))
		};
	}

	auto icon(std::string n) {
		return modify_guild_settings<Ts..., guild_settings::icon>{
			std::tuple_cat(std::move(stuff),
						   std::tuple(guild_settings::icon{std::move(n)}))
		};
	}

	auto owner_id(snowflake n) {
		return modify_guild_settings<Ts..., guild_settings::owner_id>{
			std::tuple_cat(std::move(stuff),
						   std::tuple(guild_settings::owner_id{std::move(n)}))
		};
	}

	auto splash(std::string n) {
		return modify_guild_settings<Ts..., guild_settings::splash>{
			std::tuple_cat(std::move(stuff),
						   std::tuple(guild_settings::splash{std::move(n)}))
		};
	}

	auto banner(std::string n) {
		return modify_guild_settings<Ts..., guild_settings::banner>{
			std::tuple_cat(std::move(stuff),
						   std::tuple(guild_settings::banner{std::move(n)}))
		};
	}

	auto system_channel_id(snowflake n) {
		return modify_guild_settings<Ts..., guild_settings::system_channel_id>{
			std::tuple_cat(std::move(stuff),
						   std::tuple(guild_settings::system_channel_id{std::move(n)}))
		};
	}


	std::tuple<Ts...> stuff;
};

