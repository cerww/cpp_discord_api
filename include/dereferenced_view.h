#pragma once
#include <algorithm>

template<typename T>
struct value_type{
	using value = decltype(*std::declval<T>().begin());
};

template<typename rng, std::enable_if_t<std::is_void_v<std::void_t<decltype(*std::declval<value_type<rng>>())>>, int> = 0>
struct dereferenced_view{

	template<typename R>
	explicit dereferenced_view(R&& ra):r(std::forward<R>(ra)) {}

	template<typename it_t>
	struct templated_iterator{
		templated_iterator() = default;

		explicit templated_iterator(it_t a):m_underlying_it(std::move(a)){}

		decltype(auto) operator*()const noexcept {
			return **m_underlying_it;
		}

		templated_iterator operator++() {
			auto t = *this;
			++m_underlying_it;
			return t;
		}

		templated_iterator& operator++(int) {
			++m_underlying_it;
			return *this;
		}

		template<typename other_it_t>
		bool operator==(const templated_iterator<other_it_t>& other) const{
			return m_underlying_it == other.m_underlying_it;
		}

		template<typename other_it_t>
		bool operator!=(const templated_iterator<other_it_t>& other) const {
			return m_underlying_it != other.m_underlying_it;
		}

		templated_iterator operator+(int n) const{
			return templated_iterator(m_underlying_it + n);
		}

		templated_iterator operator-(int n) const {
			return templated_iterator(m_underlying_it - n);
		}

		decltype(auto) operator-(templated_iterator o) const {
			return m_underlying_it - o.m_underlying_it;
		}

		templated_iterator& operator+=(int n) const {
			m_underlying_it += n;
			return *this;
		}

		templated_iterator& operator-=(int n) const {
			m_underlying_it -= n;
			return *this;
		}

		decltype(auto) operator[](int n)const{
			return m_underlying_it[n];
		}

	private:
		it_t m_underlying_it = {};
		template<typename>
		friend struct templated_iterator;
	};


	using iterator = templated_iterator<decltype(std::declval<rng>().begin())>;
	using const_iterator = templated_iterator<decltype(std::declval<std::add_const_t<rng>>().begin())>;

	auto begin()noexcept {
		return templated_iterator(r.begin());
	}

	auto begin() const noexcept {
		return templated_iterator(r.begin());
	}

	auto end() noexcept {
		return templated_iterator(r.end());
	}

	auto end() const noexcept{
		return templated_iterator(r.end());
	}

private:
	rng r;
};

template<typename R>
dereferenced_view(R&&)->dereferenced_view<R>;


