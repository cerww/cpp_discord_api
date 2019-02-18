#pragma once
#include <optional>


template<typename T>
struct optional_ref{
	constexpr optional_ref() = default;

	constexpr optional_ref(T& thing):m_self(&thing) {}
	constexpr optional_ref(T* thing) : m_self(thing) {}
	constexpr optional_ref(T&& thing) = delete;
	
	constexpr optional_ref(std::nullopt_t) {}
	constexpr optional_ref(std::nullptr_t) {}
	
	constexpr optional_ref& operator=(std::nullopt_t) {
		m_self = nullptr;
		return *this;
	}

	constexpr optional_ref& operator=(std::nullptr_t) {
		m_self = nullptr;
		return *this;
	}

	constexpr optional_ref& operator=(T&& other) = delete;

	constexpr optional_ref& operator=(T& other) {
		m_self = &other;
		return *this;
	}
	
	constexpr T& value() {
		if(m_self) {
			return *m_self;
		}throw std::bad_optional_access();		
	}

	constexpr const T& value()const {
		if (m_self) {
			return *m_self;
		}throw std::bad_optional_access();
	}

	constexpr bool has_value()const noexcept {
		return m_self;
	}

	constexpr T& operator*() noexcept{
		return *m_self;
	}

	constexpr const T& operator*() const noexcept {
		return *m_self;
	}

	constexpr T* operator->() noexcept{
		return m_self;
	}

	constexpr const T* operator->()const noexcept {
		return m_self;
	}

	constexpr T& emplace(T& thing) noexcept{
		m_self = &thing;
		return thing;
	}

	constexpr operator bool()const noexcept{
		return m_self;
	}

	constexpr operator T&() {
		return value();
	}

	constexpr operator const T&() const {
		return value();
	}

	constexpr void reset()noexcept {
		m_self = nullptr;
	}

	constexpr T& get() {
		return *m_self;
	}

	constexpr const T& get()const {
		return *m_self;
	}

	constexpr T& value_or(T&&) noexcept = delete;
	constexpr T& value_or(T&&) const noexcept = delete;

	constexpr T& value_or(T& other) noexcept{
		if (has_value())
			return *m_self;
		return other;
	}

	constexpr const T& value_or(const T& other)const noexcept {
		if (has_value())
			return *m_self;
		return other;
	}

	operator optional_ref<const T>()const noexcept{
		optional_ref<const T> ret(m_self);
		return ret;
	}

private:
	T* m_self = nullptr;
};

/*
template<typename T,typename U>
constexpr std::enable_if_t<has_equal_to<T,U>::value,bool> operator==(const optional_ref<T>& rhs,U&& lhs) noexcept(noexcept(*rhs == lhs)){
	return rhs.has_value() ? *rhs == lhs : false;
}

template<typename T, typename U>
constexpr std::enable_if_t<has_not_equal_to<T,U>::value,bool> operator!=(const optional_ref<T>& rhs, U&& lhs) noexcept(noexcept(*rhs != lhs)) {
	return rhs.has_value() ? *rhs != lhs : true;
}

template<typename T, typename U>
constexpr std::enable_if_t<has_less_than<T, U>::value, bool> operator<(const optional_ref<T>& rhs, U&& lhs) noexcept(noexcept(*rhs < lhs)) {
	return rhs.has_value() ? *rhs < lhs : true;
}

template<typename T, typename U>
constexpr std::enable_if_t<has_less_than_equal<T, U>::value, bool> operator<=(const optional_ref<T>& rhs, U&& lhs) noexcept(noexcept(*rhs <= lhs)) {
	return rhs.has_value() ? *rhs <= lhs : true;
}

template<typename T, typename U>
constexpr std::enable_if_t<has_greater_than<T, U>::value, bool> operator>(const optional_ref<T>& rhs, U&& lhs) noexcept(noexcept(*rhs > lhs)) {
	return rhs.has_value() ? *rhs > lhs : false;
}

template<typename T, typename U>
constexpr std::enable_if_t<has_greater_than_equal<T, U>::value, bool> operator>=(const optional_ref<T>& rhs, U&& lhs)noexcept(noexcept(*rhs >= lhs)) {
	return rhs.has_value() ? *rhs >= lhs : false;
}


template<typename T, typename U>
constexpr std::enable_if_t<has_equal_to<T, U>::value, bool> operator==(T&& rhs, const optional_ref<U>& lhs) noexcept(noexcept(rhs == *lhs)) {
	return lhs.has_value() ? rhs == *lhs : false;
}

template<typename T, typename U>
constexpr std::enable_if_t<has_not_equal_to<T, U>::value, bool> operator!=(T&& rhs, const optional_ref<U>& lhs)noexcept(noexcept(rhs != *lhs)) {
	return lhs.has_value() ? rhs != *lhs : true;
}

template<typename T, typename U>
constexpr std::enable_if_t<has_less_than<T, U>::value, bool> operator<(T&& rhs, const optional_ref<U>& lhs)noexcept(noexcept(rhs < *lhs)) {
	return lhs.has_value() ? rhs < *lhs : false;
}

template<typename T, typename U>
constexpr std::enable_if_t<has_less_than_equal<T, U>::value, bool> operator<=(T&& rhs, const optional_ref<U>& lhs)noexcept(noexcept(rhs <= *lhs)) {
	return lhs.has_value() ? rhs <= *lhs : false;
}

template<typename T, typename U>
constexpr std::enable_if_t<has_greater_than<T, U>::value, bool> operator>(T&& rhs, const optional_ref<U>& lhs)noexcept(noexcept(rhs > *lhs)) {
	return lhs.has_value() ? rhs > *lhs : true;
}

template<typename T, typename U>
constexpr std::enable_if_t<has_greater_than_equal<T, U>::value, bool> operator>=(T&& rhs, const optional_ref<U>& lhs) noexcept(noexcept(rhs >= *lhs)) {
	return lhs.has_value() ? rhs >= *lhs : true;
}

*/

inline void asdasdasd() {
	int a = 0;
	optional_ref<int> w = std::nullopt;
	w = a;
	w = nullptr;
	w = a;
	w.value() = 1;
	if(w){}
}

