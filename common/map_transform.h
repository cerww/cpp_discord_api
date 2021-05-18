#pragma once
#include <unordered_map>
#include <utility>
#include "iterator_facade.h"


template<typename T, typename F>
struct map_transform {
	map_transform(T a, F b) :
		m_map(std::forward<T>(a)),
		m_fn(std::forward<F>(b)) {}


	template<typename A>
	auto operator[](A&& a)->decltype(std::invoke(std::declval<F>(), std::declval<T>()[std::forward<A>(a)])) {
		return std::invoke(m_fn, m_map[std::forward<A>(a)]);
	}

	template<typename A>
	auto operator[](A&& a) const->decltype(std::invoke(std::declval<const F>(), std::declval<const T>()[std::forward<A>(a)])) {
		return std::invoke(m_fn, m_map[std::forward<A>(a)]);
	}

	template<typename A> requires requires(T m,A b)
	{
		m.at(b);
	}
	auto operator[](A&& a) const->decltype(std::invoke(std::declval<const F>(), std::declval<const T>().at(std::forward<A>(a)))) {
		return std::invoke(m_fn, m_map.at(std::forward<A>(a)));
	}

	template<typename A>
	auto at(A&& a)->decltype(std::invoke(std::declval<F>(), std::declval<T>().at(std::forward<A>(a)))) {
		return std::invoke(m_fn, m_map.at(std::forward<A>(a)));
	}

	template<typename A>
	auto at(A&& a) const->decltype(std::invoke(std::declval<const F>(), std::declval<const T>().at(std::forward<A>(a)))) {
		return std::invoke(m_fn, m_map.at(std::forward<A>(a)));
	}

	template<typename inner_iterator_t>
	struct cursor {
		cursor() = default;

		template<typename A = int> requires !std::is_reference_v<F>
			cursor(inner_iterator_t inner_iterator, F fn) :
				m_inner_iterator(std::move(inner_iterator)),
				m_fn(std::move(fn)) {}

			template<typename A = int> requires std::is_reference_v<F>
				cursor(inner_iterator_t inner_iterator, F fn) :
					m_inner_iterator(std::move(inner_iterator)),
					m_fn(&fn) {}

				auto read() {
					if constexpr (std::is_reference_v<F>) {
						return std::invoke(*m_fn, *m_inner_iterator);
					}
					else {
						return std::invoke(m_fn, *m_inner_iterator);
					}
				}

				void next() {
					++m_inner_iterator;
				}

				template<typename A = typename T::iterator> requires requires(A a) { --a; }
				void prev() {
					--m_inner_iterator;
				}

				bool operator==(const cursor& other) const {
					return m_inner_iterator == other.m_inner_iterator;
				}

	private:
		inner_iterator_t m_inner_iterator;
		std::conditional_t<std::is_reference_v<F>, std::remove_reference_t<F>*, F> m_fn;
	};

	using iterator = iterator_facade<cursor<typename std::remove_cvref_t<T>::iterator>>;
	using const_iterator = iterator_facade<cursor<typename std::remove_cvref_t<T>::const_iterator>>;

	auto begin() {
		return iterator(m_map.begin(), m_fn);
	}
	
	auto end() {
		return iterator(m_map.end(), m_fn);
	}

	auto begin()const {
		return const_iterator(std::as_const(m_map).begin(), m_fn);
	}

	auto end()const {
		return const_iterator(std::as_const(m_map).end(), m_fn);
	}

	template<typename K>
	auto find(K&& k) {
		return iterator(m_map.find(std::forward<K>(k)), m_fn);
	}

	template<typename K>
	auto find(K&& k)const {
		return const_iterator(std::as_const(m_map).find(std::forward<K>(k)), m_fn);
	}

	template<typename K>
	bool contains(K&& k)const {
		if constexpr (requires(const T map, K b) { map.contains(std::forward<K>(k)); }) {
			return m_map.contains(std::forward<K>(k));
		}else {
			return m_map.find(std::forward<K>(k)) != m_map.end();
		}
	}

	size_t size()const noexcept {
		return m_map.size();
	}

	bool empty()const noexcept {
		return size() == 0;
	}	

private:
	T m_map;
	F m_fn;
};

template<typename T, typename F>
map_transform(T&&, F&&)->map_transform<T, F>;
