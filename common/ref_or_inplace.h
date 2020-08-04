#pragma once
#include <variant>
#include <optional>
#include <utility>
#include <vector>
#include <stdexcept>
#include <concepts/concepts.hpp>



//maybe rename
template<typename T>
struct ref_or_inplace {

	//requires default_constructible<T>
	
	ref_or_inplace() requires std::is_default_constructible_v<T> :m_data(std::in_place_type<T>){}

	ref_or_inplace(T& t):
		m_data(&t) { }

	explicit ref_or_inplace(T* t) :
		m_data(t) { }

	ref_or_inplace(std::nullptr_t) = delete;

	ref_or_inplace(T&& t) :
		m_data(std::move(t)) { }

	template<typename... Args,std::enable_if_t<std::is_constructible_v<T,Args...>,int> = 0>
	explicit ref_or_inplace(Args&&... args):
		m_data(std::in_place_type<T>,std::forward<Args>(args)...) {
		
	}

	T& value() {
		if(std::holds_alternative<T>(m_data)) {
			return std::get<T>(m_data);
		}else if(std::holds_alternative<T*>(m_data)) {
			return *std::get<T*>(m_data);
		}else {
			throw std::runtime_error(";-;");
		}
	}
	
	const T& value() const{
		if (std::holds_alternative<T>(m_data)) {
			return std::get<T>(m_data);
		}
		else if (std::holds_alternative<T*>(m_data)) {
			return *std::get<T*>(m_data);
		}
		else {
			throw std::runtime_error(";-;");
		}
	}

	
	

private:
	std::variant<T, T*> m_data;

};
