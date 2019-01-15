#pragma once
#include <utility>


template<typename map_type>
struct map_view{
	using map_iterator_type = decltype(std::declval<const map_type>().begin());
	map_view() = default;
	map_view(const map_type& o) :m_me(&o) {}
	explicit map_view(const map_type* o) :m_me(o) {}

	template<typename T>
	decltype(auto) operator[](T&& thing)const{
		return m_me->at(std::forward<T>(thing));
	}

	template<typename T>
	decltype(auto) at(T&& thing)const {
		return m_me->at(std::forward<T>(thing));
	}

	template<typename it_type>
	struct templated_iterator{
		templated_iterator() = default;
		templated_iterator(map_iterator_type it):m_it(std::move(it)){}
		decltype(auto) operator*() const{
			return *m_it;
		}
				
		templated_iterator& operator++(int){
			++m_it;
			return *this;
		}

		templated_iterator operator++() {
			auto other = *this;
			++m_it;
			return other;
		}

		templated_iterator& operator--(int) {
			--m_it;
			return *this;
		}

		templated_iterator operator--() {
			auto other = *this;
			--m_it;
			return other;
		}

		bool operator==(const templated_iterator& other) const noexcept {
			return m_it == other.m_it;
		}

		bool operator!=(const templated_iterator& other) const noexcept{
			return m_it == other.m_it;
		}

	private:
		map_iterator_type m_it;
	};

	using iterator = templated_iterator<map_iterator_type>;
	using const_iterator = iterator;

	iterator begin() const noexcept{
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

private:
	const map_type* m_me = nullptr;
};

