#pragma once
#include <utility>

template<typename map_type>
struct map_view {
	using map_iterator_type = decltype(std::declval<const map_type>().begin());
	using key_type = std::decay_t<decltype(std::declval<map_iterator_type>()->first)>;
	using mapped_type = std::decay_t<decltype(std::declval<map_iterator_type>()->second)>;
	using value_type = std::decay_t<decltype(*std::declval<map_iterator_type>())>;
	using size_type = size_t;
	using difference_type = ptrdiff_t;

	map_view() = default;

	map_view(const map_type& o) :
		m_me(&o) {}

	explicit map_view(const map_type* o) :
		m_me(o) {}

	template<typename T>
	decltype(auto) operator[](T&& thing) const {
		return m_me->at(std::forward<T>(thing));
	}

	template<typename T>
	decltype(auto) at(T&& thing) const {
		return m_me->at(std::forward<T>(thing));
	}

	template<typename it_type>
	struct cursor {
		cursor() = default;

		cursor(map_iterator_type it):
			m_it(std::move(it)) {}

		decltype(auto) operator*() const {
			return *m_it;
		}

		cursor& operator++(int) {
			++m_it;
			return *this;
		}

		cursor operator++() {
			auto other = *this;
			++m_it;
			return other;
		}

		cursor& operator--(int) {
			--m_it;
			return *this;
		}

		cursor operator--() {
			auto other = *this;
			--m_it;
			return other;
		}

		bool operator==(const cursor& other) const noexcept {
			return m_it == other.m_it;
		}

	private:
		map_iterator_type m_it;
	};

	using iterator = cursor<map_iterator_type>;
	using const_iterator = iterator;

	iterator begin() const noexcept {
		return iterator(m_me->begin());
	}

	iterator end() const noexcept {
		return iterator(m_me->end());
	}

	const_iterator cbegin() const noexcept {
		return iterator(m_me->cbegin());
	}

	const_iterator cend() const noexcept {
		return iterator(m_me->cend());
	}

	template<typename T>
	iterator contains(T&& t) const->decltype(iterator(std::declval<map_type>().find(t))) {
		return iterator(m_me->find(std::forward<T>(t)));
	}

	template<typename T>
	auto contains(T&& t) const->decltype(std::declval<map_type>().find(t) != std::declval<map_type>().end()) {
		return m_me->find(std::forward<T>(t)) != m_me->end();
	}

	template<typename T>
	auto count(T&& t) const->decltype(std::declval<map_type>().count(std::forward<T>(t))) {
		return m_me->count(std::forward<T>(t));
	}


private:
	const map_type* m_me = nullptr;
};
