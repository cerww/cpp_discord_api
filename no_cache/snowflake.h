#pragma once
#include <nlohmann/json.hpp>
#include <utility>
#include <type_traits>
#include <charconv>
#include <fmt/core.h>
#include <fmt/format.h>

namespace cacheless {

struct snowflake {

	//constexpr std::strong_ordering operator<=>(const snowflake& other) const noexcept = default;
	constexpr bool operator==(const snowflake& other) const noexcept = default;

	snowflake() = default;

	explicit snowflake(uint64_t a):
		val(a) {}

	uint64_t val = 0;//no reason to be private?
};

inline void to_json(nlohmann::json& json, const snowflake& wawt) {
	char buffer[20] = {};
	const auto r = std::to_chars(buffer, buffer + 20, wawt.val);
	json = std::string(buffer, r.ptr);
}

inline void from_json(const nlohmann::json& in, snowflake& out) {
	if (in.is_null())
		return;
	const auto& str = in.get_ref<const std::string&>();
	std::from_chars(str.data(), str.data() + str.size(), out.val);
}


struct id_equal_to {
	explicit id_equal_to(snowflake i) :
		id(i) {}

	template<typename T>
	constexpr bool operator()(T&& thing) {
		return thing.id() == id;
	}

	const snowflake id;
};


template<typename T>
std::pair<snowflake, T> get_then_return_id(const nlohmann::json& json) {
	auto ret = json.get<T>();
	const snowflake id = ret.id;
	return {id, std::move(ret)};
}

static inline const auto id_comp = [](auto&& a, snowflake b) { return a.id() < b; };

//static inline constexpr auto get_id = [](auto&& a) { return a.id(); };


}


template<typename Char>
struct fmt::formatter<cacheless::snowflake, Char> :fmt::formatter<uint64_t, Char> {

	template<typename FormatContext>
	auto format(const cacheless::snowflake& val, FormatContext& ctx) {
		return fmt::formatter<uint64_t>::format(val.val, ctx);
	}
};


namespace std {

template<>
struct hash<cacheless::snowflake> {
	size_t operator()(cacheless::snowflake s) const noexcept {
		return std::hash<uint64_t>()(s.val);
	}
};

}
