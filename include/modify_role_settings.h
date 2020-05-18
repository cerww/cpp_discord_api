#pragma once
#include <string>
#include "permission.h"

struct role_settings {
	struct name {
		std::string n;
		static constexpr std::string_view vname = "name";
	};

	struct permissions {
		permission n;
		static constexpr std::string_view vname = "permissions";
	};

	struct color {
		int n;
		static constexpr std::string_view vname = "color";
	};

	struct hoist {
		bool n;
		static constexpr std::string_view vname = "hoist";
	};

	struct mentionable {
		bool n;
		static constexpr std::string_view vname = "mentionable";
	};
};

template<typename ...Ts>
struct modify_role_settings {

	auto name(std::string n) {
		return modify_role_settings<Ts..., role_settings::name>{
			std::tuple_cat(std::move(stuff),
				std::tuple(role_settings::name{ std::move(n) }))
		};
	}

	auto permissions(permission n) {
		return modify_role_settings<Ts..., role_settings::permissions>{
			std::tuple_cat(std::move(stuff),
				std::tuple(role_settings::permissions{ std::move(n) }))
		};
	}

	auto color(int n) {
		return modify_role_settings<Ts..., role_settings::color>{
			std::tuple_cat(std::move(stuff),
				std::tuple(role_settings::color{ std::move(n) }))
		};
	}

	auto hoist(bool n) {
		return modify_role_settings<Ts..., role_settings::hoist>{
			std::tuple_cat(std::move(stuff),
				std::tuple(role_settings::hoist{ std::move(n) }))
		};
	}

	auto mentionable(bool n) {
		return modify_role_settings<Ts..., role_settings::mentionable>{
			std::tuple_cat(std::move(stuff),
				std::tuple(role_settings::mentionable{ std::move(n) }))
		};
	}
	
	std::tuple<Ts...> stuff;
};
