#pragma once
#include <utility>
#include <memory>

struct defer_construction{};

//TODO: switch to unique_allocate once c++20 is here
template<typename T,typename Allocator = std::allocator<T>>
struct indirect {
	indirect():m_data(new(m_allocator.allocate(1)) T()){};

	indirect(T stuff, Allocator al = Allocator()) :
		m_allocator(std::move(al)),
		m_data(new(m_allocator.allocate(1)) T(std::move(stuff))) {}

	template<typename allo, std::enable_if_t<std::is_constructible_v<Allocator,allo>,int> = 0>
	explicit indirect(allo&& alloc,T thing = T()): // NOLINT
		m_allocator(std::forward<allo>(alloc)),
		m_data(new(m_allocator.allocate(1)) T(std::move(thing))){}

	template<
		typename... Ts, 
		std::enable_if_t<
			std::is_constructible_v<Allocator, Ts...> && 
			!std::is_constructible_v<T, Ts...>,
		int> = 0 >
	explicit indirect(Ts&&... args):
		m_allocator(std::forward<Ts>(args)...),
		m_data(new(m_allocator.allocate(1)) T()){}

	template<
		typename... Ts, 
		std::enable_if_t<
			std::is_constructible_v<Allocator, Ts...> && 
			!std::is_constructible_v<T, Ts...>,
		int> = 0>
	explicit indirect(defer_construction,Ts&&... args) :
		m_allocator(std::forward<Ts>(args)...){}

	template<typename... Ts, std::enable_if_t<std::is_constructible_v<T, Ts...>, int> = 0>
	explicit indirect(Ts&&... args):
		m_data(new(m_allocator.allocate(1)) T(std::forward<Ts>(args)...)){}

	template<typename ...Ts1, typename ...Ts2>
	explicit indirect(std::tuple<Ts1...> alloc_args,std::tuple<Ts2...> T_args):
		m_allocator(std::make_from_tuple<Allocator>(std::move(alloc_args))),
		m_data(new (m_allocator.allocate(1)) T(std::make_from_tuple<T>(std::move(T_args)))){}

	template<typename U>
	indirect(std::initializer_list<U> li):m_data(new(m_allocator.allocate(1)) T(li)) {}

	template<typename U>
	indirect& operator=(std::initializer_list<U> li) {
		*m_data = T(li);
		return *this;
	}
	
	indirect(const indirect& other):
		m_allocator(other.m_allocator),
		m_data(new(m_allocator.allocate(1)) T(*other.m_data)){}

	indirect(indirect&& other) noexcept: 
		m_allocator(std::move(other.m_allocator)),
		m_data(std::exchange(other.m_data,nullptr)) {}
	
	template<typename U, typename A, std::enable_if_t<!std::is_same_v<A, Allocator>, int> = 0>
	indirect(const indirect<U,A>& other) :
		m_data(new(m_allocator.allocate(1)) T(*other.m_data)){}

	template<typename U, typename A, std::enable_if_t<!std::is_same_v<A, Allocator>, int> = 0>
	indirect(indirect<U, A>&& other):
		m_data(new(m_allocator.allocate(1)) T(std::move(*other.m_data))) {}
	
	indirect& operator=(const indirect& other) {		
		*m_data = *other.m_data;
		return *this;
	}

	indirect& operator=(indirect&& other) noexcept{
		indirect new_me(std::move(other));
		std::swap(m_allocator,new_me.m_allocator);
		std::swap(m_data, new_me.m_data);
		return *this;
	}
	
	template<typename U,typename A, std::enable_if_t<std::is_convertible_v<U, T> && !std::is_same_v<A, Allocator>,int> = 0>
	indirect& operator=(const indirect<U,A>& other) {
		*m_data = *other.m_data;
		return *this;
	}

	template<typename U, typename A, std::enable_if_t<std::is_convertible_v<U, T> && !std::is_same_v<A, Allocator>, int> = 0>
	indirect& operator=(indirect<U,A>&& other) {		
		if(!m_data) 
			m_data = new(m_allocator.allocate(1)) T(std::move(*other.m_data));
		else
			*m_data = std::move(*other.m_data);
		return *this;
	}

	indirect& operator=(T other){
		if(!m_data) 
			m_data = new(m_allocator.allocate(1)) T(std::move(other));
		else
			*m_data = std::move(other);
		return *this;
	}

	~indirect(){
		if(m_data){
			std::allocator_traits<Allocator>::destroy(m_allocator, m_data);
			m_allocator.deallocate(m_data,1);
		}
	}
	
	T& value() noexcept{ return *m_data; }
	const T& value()const noexcept { return *m_data; }

	T* get()noexcept { return m_data; }
	const T* get()const noexcept { return m_data; }

	operator T&() { 
		return *m_data;	
	}
	
	operator const T&() const{
		return *m_data;
	}

	T& operator*() noexcept{ return *m_data; }
	const T& operator*()const noexcept{ return *m_data; }
	T* operator->() noexcept{ return m_data; }
	const T* operator->()const noexcept{ return m_data; }
	
	template<typename U>
	bool operator==(U&& other)const {
		return other == *m_data;
	}

	template<typename U>
	bool operator!=(U&& other)const {
		return other == *m_data;
	}
	
	template<typename U>
	bool operator<(U&& other)const {
		return std::less<>()(*m_data,other);//why do i use std::less
	}

	template<typename U>
	bool operator>=(U&& other)const {
		return !std::less<>()(*m_data, other);
	}

	template<typename U>
	bool operator>(U&& other)const {
		return std::less<>()(other, *m_data);
	}

	template<typename U>
	bool operator<=(U&& other)const {
		return !std::less<>()(other, *m_data);
	}
	
	Allocator& allocator() {
		return m_allocator;
	}

	const Allocator& allocator() const{
		return m_allocator;
	}

	template<typename ...args>
	void construct(args&&... Args) {
		m_data = new(m_allocator.allocate(1)) T(std::forward<args>(Args)...);
	}

private:
	Allocator m_allocator{};
	T* m_data = nullptr;
	template<typename,typename> friend struct indirect;
};

/*
#include <memory_resource>
#include "allocatey.h"
#include <unordered_map>

inline void qwettrsffdh() {
	auto lamby = [](int& c) {std::cout << c << std::endl; };
	auto lamby2 = [](int c) {std::cout << c << std::endl; };
	single_chunk_mem_pool e;
	indirect<int, single_chunk_allocator<int>> a(e);
	a = 12;
	indirect<int, single_chunk_allocator<int>> aa = a;

	indirect<int, single_chunk_allocator<int>> aab = std::move(aa);

	std::pmr::monotonic_buffer_resource pool;
	indirect<int, std::pmr::polymorphic_allocator<int>> qwe(&pool,2);
	lamby(qwe);
	lamby(a);
	lamby2(a);
	double y = a;
	int i = a;
	indirect<double> d = a;
	auto qwert  = a == d;
	auto qwea = d == y;

	std::unordered_map<int, indirect<int>> mapu;
	std::unordered_map<int, indirect<std::vector<int>>> mapua;
	mapua[2].value().emplace_back();

	mapu[2] = d;
	mapu[2] = 9;
	mapu.insert(std::make_pair(1, 2));
	mapu[3] = qwe;
	mapu[qwe] = aab;
	mapu.insert(std::make_pair(65, aa));

	std::vector<indirect<int>> vecy;
	vecy.push_back(d);
	vecy.emplace_back(3);
	int& lastu = vecy.emplace_back();
	vecy.erase(vecy.begin());
	std::vector<indirect<std::vector<int>>> rawrland;

	rawrland.emplace_back().value().emplace_back();
	std::vector<int> weutyhf = std::move(rawrland.back());
	rawrland.pop_back();
	rawrland.emplace_back(std::vector<int>{1, 2, 3, 4});

	std::vector<indirect<std::vector<int>>> hjk;
	hjk.push_back({1,2,3,4});
	hjk.emplace_back(std::vector<int>{1, 2, 34});
	//hjk.emplace_back({1, 2, 34});

	lamby2(std::move(d));
	d = a;
	d = a + 3;//std::move(a);
	//a = std::move(a);
	indirect<size_t> lop(a);
	indirect<double> c = a;
	indirect<double> t = c + 2;
	auto aweqasd = c < qwe + qwe > 0 +0 > a + 1;

	std::vector<int> v;
	
	v.push_back(a + y + i + d);
	std::cout << d << std::endl;
}
*/

