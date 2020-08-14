#pragma once
#include <string>
#include <vector>
#include <string_view>
#include <variant>
#include <optional>


namespace remove_me_later {
	template<typename T, typename U>
	concept convertible_to = std::is_convertible_v<T, U>;

	template<typename T, typename U>
	concept same_as = std::is_same_v<T, U>;

}

template<typename T>
struct parse_result_value {
	using value_type = std::decay_t<decltype(std::declval<T>().operator()(std::declval<std::string_view>()).value())>;
};

template<typename T>
using parse_result_value_t = std::decay_t<decltype(std::declval<T>().operator()(std::declval<std::string_view>()).value())>;


template<typename T>
struct parse_result;

struct parse_fail_with_reason {
	parse_fail_with_reason() = default;

	explicit parse_fail_with_reason(std::string r) :
		reason(std::move(r)) {}

	explicit parse_fail_with_reason(std::string_view r) :
		reason(std::string(r)) {}

	explicit parse_fail_with_reason(const char* r) :
		reason(std::string(r)) {}
	
	std::string reason = "";
};

template<typename T>
struct propagate_parse_fail {
	parse_result<T>* failed_parse_result = nullptr;
};

template<typename T>
struct propagate_parse_fail_const {
	const parse_result<T>* failed_parse_result = nullptr;
};

template<typename T>
struct propagate_parse_fail_r_value {
	parse_result<T>&& failed_parse_result;
	//parse_result<T> failed_parse_result;
};

struct parse_fail_no_reason {};

[[nodiscard]]
constexpr parse_fail_no_reason parse_fail() noexcept {
	return {};
}

[[nodiscard]]
inline parse_fail_with_reason parse_fail(std::string reason) {
	return parse_fail_with_reason(std::move(reason));
}

template<typename T>
concept parse_result_c = requires(T r) {
	{r.success()}->remove_me_later::convertible_to<bool>;
	{r.rest()}->remove_me_later::same_as<std::string_view>;
	{r.value()};
}&& std::is_constructible_v<T, parse_fail_no_reason>
&& std::is_constructible_v<T, parse_fail_with_reason>
&& std::is_constructible_v<T, propagate_parse_fail_r_value<T>>
&& std::is_constructible_v<T, propagate_parse_fail<T>>
&& std::is_constructible_v<T, propagate_parse_fail_const<T>>;

template<typename T>
concept parser = requires(T p) {
	{p.operator()(std::string_view())}->parse_result_c;
};


template<typename T>
struct parse_result {
	using parse_value = T;

	constexpr parse_result() = default;

	constexpr explicit parse_result(T a, std::string_view s) :
		m_self(std::in_place_index<0>, std::make_pair(std::move(a), s)) {}

	constexpr parse_result(parse_fail_no_reason a) :
		m_self(a) {}

	parse_result(parse_fail_with_reason r) :
		m_self(std::move(r.reason)) {}

	template<typename U>
	parse_result(propagate_parse_fail<U> f) {
		if (f.failed_parse_result->m_self.index() == 1) {
			m_self = std::move(std::get<1>(f.failed_parse_result->m_self));
		}
		else {
			m_self = parse_fail_no_reason();
		}
	}

	template<typename U>
	parse_result(propagate_parse_fail_const<U> f) {
		if (f.failed_parse_result->m_self.index() == 1) {
			m_self = std::get<1>(f.failed_parse_result->m_self);
		}
		else {
			m_self = parse_fail_no_reason();
		}
	}

	template<typename U>
	parse_result(propagate_parse_fail_r_value<U> f) {
		if (f.failed_parse_result.m_self.index() == 1) {
			m_self = std::move(std::get<1>(f.failed_parse_result.m_self));
		}
		else {
			m_self = parse_fail_no_reason();
		}
	}

	constexpr explicit operator bool() const noexcept {
		return success();
	}

	//TODO: remove these 4 functions eventually, maybe?
	constexpr std::pair<T, std::string_view>* operator->() {
		return &(**this);
	}

	constexpr const std::pair<T, std::string_view>* operator->() const noexcept {
		return &(**this);
	}

	constexpr auto& operator*()  {
		assert(m_self.index() == 0);
		return std::get<0>(m_self);
	}

	constexpr const auto& operator*() const noexcept {
		assert(m_self.index() == 0);
		return std::get<0>(m_self);
	}

	[[nodiscard]] constexpr bool success() const noexcept {
		return std::holds_alternative<std::pair<T, std::string_view>>(m_self);
	}

	[[nodiscard]] constexpr bool failed() const noexcept {
		return !std::holds_alternative<std::pair<T, std::string_view>>(m_self);
	}

	constexpr T& value() {
		assert(m_self.index() == 0);
		return std::get<0>(m_self).first;
	}

	[[nodiscard]] constexpr const T& value() const {
		assert(m_self.index() == 0);
		return std::get<0>(m_self).first;
	}

	[[nodiscard]] constexpr std::string_view rest() const {
		assert(m_self.index() == 0);
		return std::get<0>(m_self).second;
	}

	[[nodiscard]] constexpr std::string_view fail_reason() const {
		if (m_self.index() == 1) {
			return std::get<std::string>(m_self);
		}
		else {
			return "";
		}
	}

	//*
	template<typename U, std::enable_if_t<std::is_convertible_v<T, U>, int> = 0>
	operator parse_result<U>() const& {
		if (bool(*this)) {
			return parse_result<U>(U(value()), rest());
		}
		else if (std::holds_alternative<std::string>(m_self)) {
			return parse_fail(std::get<std::string>(m_self));
		}
		else if (std::holds_alternative<parse_fail_no_reason>(m_self)) {
			return parse_fail();
		}
		else {
			return parse_result<U>();
		}
	}

	template<typename U, std::enable_if_t<std::is_convertible_v<T, U>, int> = 0>
	operator parse_result<U>()& {
		if (bool(*this)) {
			return parse_result<U>(U(value()), rest());
		}
		else if (std::holds_alternative<std::string>(m_self)) {
			return parse_fail(std::get<std::string>(m_self));
		}
		else if (std::holds_alternative<parse_fail_no_reason>(m_self)) {
			return parse_fail();
		}
		else {
			return parse_result<U>();
		}
	}

	template<typename U, std::enable_if_t<std::is_convertible_v<T, U>, int> = 0>
	operator parse_result<U>()&& {
		if (bool(*this)) {
			return parse_result<U>(U(std::move(value())), rest());
		}
		else if (std::holds_alternative<std::string>(m_self)) {
			return parse_fail(std::move(std::get<std::string>(m_self)));
		}
		else if (std::holds_alternative<parse_fail_no_reason>(m_self)) {
			return parse_fail();
		}
		else {
			return parse_result<U>();
		}
	}

	template<typename U, std::enable_if_t<std::is_convertible_v<T, U>, int> = 0>
	operator parse_result<U>() const&& {
		if (bool(*this)) {
			return parse_result<U>(U(value()), rest());
		}
		else if (std::holds_alternative<std::string>(m_self)) {
			return parse_fail(std::get<std::string>(m_self));
		}
		else if (std::holds_alternative<parse_fail_no_reason>(m_self)) {
			return parse_fail();
		}
		else {
			return parse_result<U>();
		}
	}

	template<typename Fn> requires std::is_invocable_v<Fn,T>
	parse_result<std::invoke_result_t<Fn, T>> transform(Fn&& fn) {
		if (!success()) {
			return parse_fail(*this);
		}
		else {
			return parse_result<std::invoke_result_t<Fn, T>>(std::invoke(fn, std::move(value())), rest());
		}
	}

	template<typename Fn>
	parse_result<decltype(std::apply(std::declval<Fn>(),std::declval<T>()))> apply_transform(Fn&& fn) {
		if (!success()) {
			return parse_fail(*this);
		}
		else {
			return parse_result<decltype(std::apply(std::declval<Fn>(), std::declval<T>()))> (std::apply(fn, std::move(value())), rest());
		}
	}

	template<typename Fn>// requires std::is_invocable_v<Fn,T>
	auto and_then(Fn&& fn)->parse_result<decltype(*std::declval<std::invoke_result_t<Fn, T>>())> {
		if(failed()) {
			return parse_fail(*this);
		}else {
			auto temp = std::invoke(fn, std::move(value()));
			if(!temp) {
				return parse_fail(*this);
			}else {
				return parse_result<decltype(*std::declval<std::invoke_result_t<Fn, T>>())>(std::move(*temp),rest());
			}
			
		}
	}

	template<typename Fn> requires std::is_invocable_v<Fn,T>
	auto and_then_apply(Fn&& fn)
		->parse_result<decltype(*std::apply(fn,std::declval<T>()))> {
		if(failed()) {
			return parse_fail(*this);
		}else {
			auto temp = std::invoke(fn, std::move(value()));
			if(!temp) {
				return parse_fail(*this);
			}else {
				return parse_result<decltype(*std::apply(fn, std::declval<T>()))>(std::move(*temp),rest());
			}
			
		}
	}

	template<typename parser>
	auto then_parse(parser&& p)
		->parse_result<decltype(std::tuple_cat(
			std::declval<T>(),
			std::make_tuple(p(std::string_view()).value())
		))>
	{		
		if(failed()) {
			return parse_fail(*this);
		}

		auto r = p(rest());
		if(!r) {
			return parse_fail(r);
		}
		return std::tuple_cat(std::move(value()), std::make_tuple(std::move(r.value())));		
	}

	

	//*/
private:
	/* parse_fail_no_reason is so it'll work in constexpr, std::string isn't constexpr*/
	//why do i have std::monostate here ;-;
	std::variant<std::pair<T, std::string_view>, std::string, parse_fail_no_reason, std::monostate> m_self = std::monostate{};
	template<typename>
	friend struct parse_result;
};

template<>
struct parse_result<void> {
	using parse_value = void;

	constexpr parse_result() = default;

	constexpr explicit parse_result(std::string_view s) :
		m_self(std::in_place_index<0>, s) {}

	template<typename T, std::enable_if_t<!std::is_void_v<T>, int> = 0>
	constexpr parse_result(const parse_result<T>& other) {
		if (other) {
			m_self = other.rest();
		}
		else if (std::holds_alternative<parse_fail_no_reason>(other.m_self)) {
			m_self = parse_fail_no_reason();
		}
		else {
			m_self = std::get<std::string>(other.m_self);
		}
	}

	template<typename T, std::enable_if_t<!std::is_void_v<T>, int> = 0>
	constexpr parse_result(parse_result<T>&& other) {
		if (other) {
			m_self = other.rest();
		}
		else if (std::holds_alternative<parse_fail_no_reason>(other.m_self)) {
			m_self = parse_fail_no_reason();
		}
		else {
			m_self = std::get<std::string>(std::move(other.m_self));
		}
	}

	constexpr parse_result(parse_fail_no_reason a) :
		m_self(a) {}

	parse_result(parse_fail_with_reason r) :
		m_self(std::in_place_index<1>, std::move(r.reason)) {}

	template<typename U>
	parse_result(propagate_parse_fail<U> f) {
		if (f.failed_parse_result->m_self.index() == 1) {
			m_self = std::move(std::get<1>(f.failed_parse_result->m_self));
		}
		else {
			m_self = parse_fail_no_reason();
		}
	}

	template<typename U>
	parse_result(propagate_parse_fail_const<U> f) {
		if (f.failed_parse_result->m_self.index() == 1) {
			m_self = std::get<1>(f.failed_parse_result->m_self);
		}
		else {
			m_self = parse_fail_no_reason();
		}
	}

	template<typename U>
	parse_result(propagate_parse_fail_r_value<U> f) {
		if (f.failed_parse_result->m_self.index() == 1) {
			m_self = std::move(std::get<1>(f.failed_parse_result->m_self));
		}
		else {
			m_self = parse_fail_no_reason();
		}
	}

	constexpr explicit operator bool() const noexcept {
		return success();
	}

	constexpr const auto& operator*() const {
		return std::get<0>(m_self);
	}

	constexpr const std::string_view* operator->() const {
		return &(**this);
	}

	[[nodiscard]] constexpr bool success() const noexcept {
		return std::holds_alternative<std::string_view>(m_self);
	}

	constexpr std::string_view rest() const {
		return std::get<0>(m_self);
	}

	std::string_view fail_reason() const noexcept {
		if (m_self.index() == 1) {
			return std::get<1>(m_self);
		}
		else {
			return "";
		}
	}

private:
	std::variant<std::string_view, std::string, parse_fail_no_reason, std::monostate> m_self = std::monostate{};
	template<typename>
	friend struct parse_result;
};

//this might be dangerous
template<typename T>
[[nodiscard]]
inline propagate_parse_fail<T> parse_fail(parse_result<T>& p) {
	assert(!p.success());
	return propagate_parse_fail<T>{&p};
}

template<typename T>
[[nodiscard]]
inline propagate_parse_fail_const<T> parse_fail(const parse_result<T>& p) {
	assert(!p.success());
	return propagate_parse_fail_const<T>{&p};
}

template<typename T>
[[nodiscard]]
inline propagate_parse_fail_r_value<T> parse_fail(parse_result<T>&& p) {
	assert(!p.success());
	return propagate_parse_fail_r_value<T>{std::move(p)};
}






static_assert(parse_result_c<parse_result<int>>);