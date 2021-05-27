#pragma once
#include <memory>
#include <utility>
#include <tuple>
#include <vector>
#include <iostream>
#include "meta_fns.h"

namespace hof {
template<typename T, typename comp>
struct comparator {
	constexpr explicit comparator(T a) :
		value(std::move(a)) {}

	constexpr explicit comparator(T a, comp b) :
		value(std::move(a)), c(std::move(b)) {}

	template<typename U>
	constexpr bool operator()(U&& other) {
		return std::invoke(c, std::forward<U>(other), value);
	}

	template<typename U>
	constexpr bool operator()(U&& other) const {
		return std::invoke(c, std::forward<U>(other), value);
	}

	T value = {};
	comp c = {};
};

template<typename T>
struct is_less_than {
	template<typename TT, std::enable_if_t<!std::is_same_v<std::decay_t<TT>, is_less_than<T>>, int>  = 0>
	constexpr explicit is_less_than(TT&& a) :
		value(std::forward<TT>(a)) {}

	template<typename U>
	constexpr bool operator()(U&& other) {
		return other < value;
	}

	template<typename U>
	constexpr bool operator()(U&& other) const {
		return other < value;
	}

	T value;
};

template<typename TT>
is_less_than(TT&&)->is_less_than<TT>;

template<typename T>
struct is_greater_than {

	template<typename TT, std::enable_if_t<!std::is_same_v<std::decay_t<TT>, is_greater_than<T>>, int>  = 0>
	constexpr explicit is_greater_than(TT&& a) :
		value(std::forward<TT>(a)) {}

	template<typename U>
	constexpr bool operator()(U&& other) {
		return other > value;
	}

	template<typename U>
	constexpr bool operator()(U&& other) const {
		return other > value;
	}

	T value;
};

template<typename TT>
is_greater_than(TT&&)->is_greater_than<TT>;

template<typename T>
struct is_less_than_eq {
	template<typename TT, std::enable_if_t<!std::is_same_v<std::decay_t<TT>, is_less_than_eq<T>>, int>  = 0>
	constexpr explicit is_less_than_eq(TT&& a) :
		value(std::forward<TT>(a)) {}

	template<typename U>
	constexpr bool operator()(U&& other) {
		return other <= value;
	}

	template<typename U>
	constexpr bool operator()(U&& other) const {
		return other <= value;
	}

	T value;
};

template<typename TT>
is_less_than_eq(TT&&)->is_less_than_eq<TT>;

template<typename T>
struct is_greater_than_eq {
	template<typename TT, std::enable_if_t<!std::is_same_v<std::decay_t<TT>, is_greater_than_eq<T>>, int>  = 0>
	constexpr explicit is_greater_than_eq(TT&& a) :
		value(std::forward<TT>(a)) {}

	template<typename U>
	constexpr bool operator()(U&& other) {
		return other >= value;
	}

	template<typename U>
	constexpr bool operator()(U&& other) const {
		return other >= value;
	}

	T value;
};

template<typename TT>
is_greater_than_eq(TT&&)->is_greater_than_eq<TT>;

template<typename T>
struct is_equal_to {
	template<typename TT, std::enable_if_t<!std::is_same_v<std::decay_t<TT>, is_equal_to<T>>, int>  = 0>
	constexpr explicit is_equal_to(TT&& a) :
		value(std::forward<TT>(a)) {}

	template<typename U>
	constexpr bool operator()(U&& other) {
		return other == value;
	}

	template<typename U>
	constexpr bool operator()(U&& other) const {
		return other == value;
	}

	T value;
};

template<typename TT>
is_equal_to(TT&&)->is_equal_to<TT>;

template<typename T>
struct is_not_equal_to {
	template<typename TT, std::enable_if_t<!std::is_same_v<std::decay_t<TT>, is_not_equal_to<T>>, int>  = 0>
	constexpr explicit is_not_equal_to(TT&& a) :
		value(std::forward<TT>(a)) {}

	template<typename U>
	constexpr bool operator()(U&& other) {
		return other != value;
	}

	template<typename U>
	constexpr bool operator()(U&& other) const {
		return other != value;
	}

	T value;
};

template<typename TT>
is_not_equal_to(TT&&)->is_not_equal_to<TT>;

//f1(f2(f3(args...)))
//f1<-f2<-f3<-args...
template<typename First, typename ...Rest>
struct compose :private compose<Rest...> {
	constexpr compose() = default;

	template<typename F, typename... R>
	constexpr explicit compose(F&& f, R&& ... rest) :
		compose<Rest...>(std::forward<R>(rest)...),
		first_fn(std::forward<F>(f)) {}

	template<typename ...Args>
	constexpr auto operator()(Args&& ... args)
	->decltype(std::invoke(std::declval<First>(), static_cast<compose<Rest...>&>(*this)(std::forward<Args>(args)...))) {

		return std::invoke(first_fn, static_cast<compose<Rest...>&>(*this)(std::forward<Args>(args)...));
	}

	template<typename ...Args>
	constexpr auto operator()(Args&& ... args) const
	->decltype(std::invoke(std::declval<const First>(), static_cast<const compose<Rest...>&>(*this)(std::forward<Args>(args)...))) {

		return std::invoke(first_fn, static_cast<compose<Rest...>&>(*this)(std::forward<Args>(args)...));
	}

	template<typename ...Args> requires !std::is_invocable_v<First, std::invoke_result_t<const compose<Rest...>, Args...>>
	constexpr auto operator()(Args&& ... args) const
	->decltype(std::apply(std::declval<const First>(), static_cast<const compose<Rest...>&>(*this)(std::forward<Args>(args)...))) {

		return std::apply(first_fn, static_cast<const compose<Rest...>&>(*this)(std::forward<Args>(args)...));
	}

	template<typename ...Args>requires !std::is_invocable_v<First, std::invoke_result_t<compose<Rest...>, Args...>>
	constexpr auto operator()(Args&& ... args)
	->decltype(std::apply(std::declval<First>(), static_cast<compose<Rest...>&>(*this)(std::forward<Args>(args)...))) {

		return std::apply(first_fn, static_cast<compose<Rest...>&>(*this)(std::forward<Args>(args)...));
	}

	First first_fn = {};
};

template<typename Last>
struct compose<Last> {
	constexpr compose() = default;

	template<typename L> requires !std::is_same_v<std::decay<L>, compose<Last>>
	constexpr explicit compose(L&& last) :
		last_fn(std::forward<L>(last)) {}

	template<typename... Args> requires std::is_invocable_v<Last, Args...>
	constexpr auto operator()(Args&& ... args)->decltype(std::invoke(std::declval<Last>(), std::forward<Args>(args)...)) {

		return std::invoke(last_fn, std::forward<Args>(args)...);
	}

	template<typename... Args>requires std::is_invocable_v<const Last, Args...>
	constexpr auto operator()(Args&& ... args) const->decltype(std::invoke(std::declval<const Last>(), std::forward<Args>(args)...)) {

		return std::invoke(last_fn, std::forward<Args>(args)...);
	}

	template<typename ...Args>requires !std::is_invocable_v<Last, Args...>
	constexpr auto operator()(Args&& ... args) const->decltype(std::apply(std::declval<Last>(), std::forward<Args>(args)...)) {

		return std::apply(last_fn, std::forward<Args>(args)...);
	}

	template<typename ...Args>requires !std::is_invocable_v<const Last, Args...>
	constexpr auto operator()(Args&& ... args) const->decltype(std::apply(std::declval<const Last>(), std::forward<Args>(args)...)) {

		return std::apply(last_fn, std::forward<Args>(args)...);
	}

	Last last_fn = {};
};

template<typename F, typename... R>
compose(F&&, R&&...)->compose<F, R...>;

template<typename L>
compose(L&&)->compose<L>;

//f3(f2(f1(args...)))
//args...->f1->f2->f3
template<typename First, typename ...Rest>
struct flow :private flow<Rest...> {
	constexpr flow() = default;

	template<typename F, typename... R>
	constexpr explicit flow(F&& f, R&& ... rest) :
		flow<Rest...>(std::forward<R>(rest)...),
		first_fn(std::forward<F>(f)) {}

	template<typename ...Args>
	constexpr auto operator()(Args&& ... args)
	->decltype(static_cast<flow<Rest...>&>(*this)(std::invoke(std::declval<First>(), std::forward<Args>(args)...))) {

		return static_cast<flow<Rest...>&>(*this)(std::invoke(first_fn, std::forward<Args>(args)...));
	}

	template<typename ...Args>
	constexpr auto operator()(Args&& ... args) const
	->decltype(static_cast<const flow<Rest...>&>(*this)(std::invoke(std::declval<const First>(), std::forward<Args>(args)...))) {

		return static_cast<const flow<Rest...>&>(*this)(std::invoke(first_fn, std::forward<Args>(args)...));
	}

	template<typename ...Args>requires !std::invocable<First, Args...>
	constexpr auto operator()(Args&& ... args)
	->decltype(static_cast<flow<Rest...>&>(*this)(std::apply(std::declval<First>(), std::forward<Args>(args)...))) {

		return static_cast<flow<Rest...>&>(*this)(std::apply(first_fn, std::forward<Args>(args)...));
	}

	template<typename ...Args> requires !std::invocable<const First, Args...>
	constexpr auto operator()(Args&& ... args) const
	->decltype(static_cast<const flow<Rest...>&>(*this)(std::apply(std::declval<const First>(), std::forward<Args>(args)...))) {

		return static_cast<const flow<Rest...>&>(*this)(std::apply(first_fn, std::forward<Args>(args)...));
	}

	First first_fn = {};
};

template<typename Last>
struct flow<Last> {
	constexpr flow() = default;

	template<typename L> requires !std::same_as<std::decay_t<L>, flow<Last>>
	constexpr explicit flow(L&& last) :
		last_fn(std::forward<L>(last)) {}

	template<typename... Args> requires std::invocable<Last, Args...>
	constexpr auto operator()(Args&& ... args)
	->decltype(std::invoke(std::declval<Last>(), std::forward<Args>(args)...)) {

		return std::invoke(last_fn, std::forward<Args>(args)...);
	}

	template<typename... Args> requires std::invocable<const Last, Args...>
	constexpr auto operator()(Args&& ... args) const
	->decltype(std::invoke(std::declval<const Last>(), std::forward<Args>(args)...)) {

		return std::invoke(last_fn, std::forward<Args>(args)...);
	}

	template<typename ...Args>requires !std::invocable<Last, Args...>
	constexpr auto operator()(Args&& ... args)
	->decltype(std::apply(std::declval<Last>(), std::forward<Args>(args)...)) {

		return std::apply(last_fn, std::forward<Args>(args)...);
	}

	template<typename ...Args>requires !std::invocable<const Last, Args...>
	constexpr auto operator()(Args&& ... args) const
	->decltype(std::apply(std::declval<const Last>(), std::forward<Args>(args)...)) {

		return std::apply(last_fn, std::forward<Args>(args)...);
	}

	Last last_fn = {};
};

template<typename F, typename... R>
flow(F&&, R&&...)->flow<F, R...>;

template<typename L>
flow(L&&)->flow<L>;

template<typename ...fns>
struct logical_conjunction {
	static_assert(sizeof...(fns) >= 1);
	constexpr logical_conjunction() = default;

	template<typename... Fns>
	constexpr explicit logical_conjunction(Fns&&... funcs) :
		functions(std::forward<Fns>(funcs)...) {}

	template<typename ...Args>
	constexpr decltype(auto) operator()(Args&&... args) {
		if constexpr (sizeof...(fns) == 1) {
			return std::invoke(std::get<0>(functions), std::forward<Args>(args)...);
		} else {
			auto tuple = std::forward_as_tuple(std::forward<Args>(args)...);
			return std::apply([&](auto&& ... funcs) {
				return (std::apply(funcs, tuple) && ...);
			}, functions);
		}
	}

	template<typename ...Args>
	constexpr decltype(auto) operator()(Args&&... args) const {
		if constexpr (sizeof...(fns) == 1) {
			return std::invoke(std::get<0>(functions), std::forward<Args>(args)...);
		} else {
			auto tuple = std::forward_as_tuple(std::forward<Args>(args)...);
			return std::apply([&](auto&& ... funcs) {
				return (std::apply(funcs, tuple) && ...);
			}, functions);
		}

	}

private:
	std::tuple<fns...> functions = {};
};

template<typename... Fns>
logical_conjunction(Fns&&...)->logical_conjunction<Fns...>;

template<typename ...fns>
struct logical_disjunction {
	static_assert(sizeof...(fns) >= 1);
	constexpr logical_disjunction() = default;

	template<typename... Fns>
	constexpr explicit logical_disjunction(Fns&&... funcs) :
		functions(std::forward<Fns>(funcs)...) {}

	template<typename ...Args>
	constexpr decltype(auto) operator()(Args&&... args) {
		if constexpr (sizeof...(fns) == 1) {
			return std::invoke(std::get<0>(functions), std::forward<Args>(args)...);
		} else {
			auto tuple = std::forward_as_tuple(std::forward<Args>(args)...);
			return std::apply([&](auto&&... funcs) {
				return (std::apply(funcs, tuple) || ...);
			}, functions);
		}
	}

	template<typename ...Args>
	constexpr decltype(auto) operator()(Args&&... args) const {
		if constexpr (sizeof...(fns) == 1) {
			return std::invoke(std::get<0>(functions), std::forward<Args>(args)...);
		} else {
			auto tuple = std::forward_as_tuple(std::forward<Args>(args)...);
			return std::apply([&](auto&& ... funcs) {
				return (std::apply(funcs, tuple) || ...);
			}, functions);
		}
	}

private:
	std::tuple<fns...> functions = {};
};

template<typename... Fns>
logical_disjunction(Fns&&...)->logical_disjunction<Fns...>;

template<typename... Fns>
using logical_or = logical_disjunction<Fns...>;

template<typename fn>
struct logical_negate_t {
	constexpr logical_negate_t() = default;

	template<typename F>
	constexpr explicit logical_negate_t(F&& a) :
		base(std::forward<F>(a)) {}

	template<typename ...T>
	constexpr auto operator()(T&&...a)->decltype(!std::invoke(std::declval<fn>(), std::forward<T>(a)...)) {
		return !std::invoke(base, std::forward<T>(a)...);
	}

	template<typename ...T>
	constexpr auto operator()(T&& ...a) const->decltype(!std::invoke(std::declval<const fn>(), std::forward<T>(a)...)) {
		return !std::invoke(base, std::forward<T>(a)...);
	}

private:
	fn base = {};
};

template<typename F>
logical_negate_t(F&&)->logical_negate_t<F>;

template<typename F, std::enable_if_t<!metap::is_specialization_of_v<F, logical_negate_t>, int>  = 0>
constexpr decltype(auto) logical_negate(F&& a) {
	return logical_negate_t(std::forward<F>(a));
}

//only move is specialized cuz ownership of underlying function can change 
//for references, if refernces are specialized, the following won't compile
/*	
auto a = []()mutable{return true;}
const auto b = logical_negate(a);//b is callable, b doesn't own a
const auto c = logical_negate(b); <--- copies a
c(); <--- won't compile 
*/
template<typename F>
constexpr decltype(auto) logical_negate(logical_negate_t<F>&& a) {
	return std::move(a.base);
}

template<typename F>
constexpr decltype(auto) logical_negate(const logical_negate_t<F>& a) {
	return logical_negate_t<const logical_negate_t<F>&>(a);
}

template<typename F>
constexpr decltype(auto) logical_negate(logical_negate_t<F>& a) {
	return logical_negate_t<logical_negate_t<F>&>(a);
}

template<typename map_type> 
struct map_with {

	template<typename map_type_t>
	constexpr explicit map_with(map_type_t&& m) :
		map(std::forward<map_type_t>(m)) {}

	template<typename T>
	constexpr decltype(auto) operator()(T&& i) {
		return map[std::forward<T>(i)];
	}

	template<typename T>
	constexpr decltype(auto) operator()(T&& i) const {
		return map[std::forward<T>(i)];
	}

private:
	map_type map;
};

template<typename T>
map_with(T&&)->map_with<T>;

struct dereference_t {
	template<typename T>
	constexpr auto operator()(T&& t) const noexcept(noexcept(*t))->decltype(*t) {
		return *t;
	}
};

constexpr static dereference_t dereference = {};

struct dereference_const_t {
	template<typename T>
	constexpr auto operator()(T&& t) const noexcept(noexcept(*t))->const auto& {
		return *t;
	}
};

constexpr static dereference_const_t dereference_const = {};

static constexpr auto always = [](auto&& a) {
	return [b = std::forward<decltype(a)>(a)](auto&&...)->decltype(auto) {
		return b;
	};
};

static constexpr auto flip_args = [](auto&& fn) {
	return [fn_ = std::forward<decltype(fn)>(fn)](auto&& a, auto&& b)->decltype(auto) {
		return std::invoke(fn_, std::forward<decltype(b)>(b), std::forward<decltype(a)>(a));
	};
};

static constexpr auto square = [](auto&& a) {
	return a * a;
};

static constexpr auto cube = [](auto&& a) {
	return a * a * a;
};

template<typename fn, typename Arg1>
struct bind1st {

	template<typename F, typename A>
	constexpr bind1st(F&& f, A&& a) :
		func(std::forward<F>(f)),
		arg(std::forward<A>(a)) {}

	template<typename... Args>
	constexpr decltype(auto) operator()(Args&&... args) const {
		return std::invoke(func, arg, std::forward<Args>(args)...);
	}

	template<typename... Args>
	constexpr decltype(auto) operator()(Args&&... args) {
		return std::invoke(func, arg, std::forward<Args>(args)...);
	}

	fn func;
	Arg1 arg;
};

template<typename F, typename A>
bind1st(F&&, A&&)->bind1st<F, A>;

template<typename fn, typename Arg2>
struct bind2nd {

	template<typename F, typename A>
	constexpr bind2nd(F&& f, A&& a) :
		func(std::forward<F>(f)), arg2(std::forward<A>(a)) {}

	template<typename Arg1>
	constexpr decltype(auto) operator()(Arg1&& arg) {
		return std::invoke(func, std::forward<Arg1>(arg), arg2);
	}

	template<typename Arg1>
	constexpr decltype(auto) operator()(Arg1&& arg) const {
		return std::invoke(func, std::forward<Arg1>(arg), arg2);
	}

	fn func;
	Arg2 arg2;
};

template<typename F, typename A>
bind2nd(F&&, A&&)->bind2nd<F, A>;

template<typename Fn, typename Argl>
struct bind_last {

	constexpr bind_last() = default;

	template<typename F, typename A>
	constexpr bind_last(F&& f, A&& a) :
		fn(std::forward<F>(f)), arg(std::forward<A>(a)) {}

	template<typename... Args>
	auto operator()(Args&&... args) const->std::invoke_result_t<Fn, Args..., Argl> {
		return std::invoke(fn, std::forward<Args>(args)..., arg);
	}

	template<typename... Args>
	auto operator()(Args&&... args)->std::invoke_result_t<Fn, Args..., Argl> {
		return std::invoke(fn, std::forward<Args>(args)..., arg);
	}

	Fn fn;
	Argl arg;
};

template<typename F, typename A>
bind_last(F&&, A&&)->bind_last<F, A>;

/*
constexpr auto bind = [](auto&& fn,auto&&... rest) {
	return[fn_ = std::forward<decltype(fn)>(fn)](auto&&... args) {
		
	};
};
*/

template<int n>
struct bind_tag {};

template<typename fn, typename... bind_args>
struct bind_first_n {
	constexpr bind_first_n() = default;

	template<typename... bind_args_>
	constexpr explicit bind_first_n(fn&& f, bind_args_&&... args):
		me(std::forward<fn>(f)),
		calling_args(std::forward<bind_args_>(args)...) {}

	template<typename ...Args>
	constexpr decltype(auto) operator()(Args&&... args) {
		return std::apply(me, std::tuple_cat(calling_args, std::forward_as_tuple(std::forward<Args>(args)...)));
	}

	template<typename ...Args>
	constexpr decltype(auto) operator()(Args&&... args) const {
		return std::apply(me, std::tuple_cat(calling_args, std::forward_as_tuple(std::forward<Args>(args)...)));
	}

	fn me;
	std::tuple<bind_args...> calling_args;
};

template<typename fn, typename ...bind_args>
bind_first_n(fn&&, bind_args&&...)->bind_first_n<fn, bind_args...>;

template<typename fn, typename transform_fn>
struct transform_args {
	constexpr transform_args() = default;

	template<typename Fn, typename Transformer>
	constexpr transform_args(Fn&& f, Transformer&& t):
		base(std::forward<Fn>(f)),
		transform(std::forward<Transformer>(t)) {}

	template<typename... Args>
	constexpr decltype(auto) operator()(Args&&... args) {
		return std::invoke(base, std::invoke(transform, std::forward<Args>(args))...);
	}

	template<typename... Args>
	constexpr decltype(auto) operator()(Args&&... args) const {
		return std::invoke(base, std::invoke(transform, std::forward<Args>(args))...);
	}

	fn base;
	transform_fn transform;
};

template<typename F, typename T>
transform_args(F&&, T&&)->transform_args<F, T>;

template<auto fn>
const auto to_functor = [](auto&&... args) {
	std::invoke(fn, std::forward<decltype(args)>(args)...);
};

constexpr inline auto swap_with2 = [](auto&& thing) {
	return [t = std::forward<decltype(thing)>(thing)](auto& other)mutable {
		std::swap(other, t);
	};
};

template<typename T>
struct swap_with {
	swap_with() = delete;

	explicit swap_with(T a):
		current_thing(std::forward<T>(a)) {}

	template<typename U>
	void operator()(U& a) {
		std::swap(a, current_thing);
	}

	T current_thing;
};

template<typename T>
swap_with(T&&)->swap_with<T>;

template<typename fn>
struct apply_transform {
	apply_transform(fn a) :
		f(std::forward<fn>(a)) { }

	template<typename Tuple_t>
	auto operator()(Tuple_t&& t)->decltype(std::apply(std::declval<fn>(), t)) {
		return std::apply(f, t);
	}

	fn f;
};

template<typename T>
apply_transform(T&&)->apply_transform<T>;

inline void asdasdasd() {
	auto i = flow([]() { return 1; }, [](int u) { return 5 + u; });
	auto t = i();
	auto qweds = compose([](int) {}, [](int a) { return 2; });
	qweds(2);
	auto fnsasd = logical_conjunction(i, []() { return false; });
	auto asdcx = logical_disjunction(fnsasd, always(true));
	asdcx();
	std::vector<int> aaa;
	auto qwe = map_with(std::move(aaa));
	auto& asdasy = qwe(2);
}

inline void ghjuasdg() {
	auto fn = []() mutable { return false; };
	const auto negated = logical_negate(fn);
	const auto negated2x = logical_negate(negated);
	std::cout << !fn() << "\n";
	std::cout << negated() << '\n';
	std::cout << !negated2x() << std::endl;
	auto negatedrawr = logical_negate([]() mutable { return false; });
	const auto negatedrawr2 = logical_negate(negatedrawr);

	auto qwe = negatedrawr();
	qwe = negatedrawr2();
	//static_assert(!fn());
	//static_assert(negated());
	//static_assert(!negated2x());
}


}

#include <tuple>
#include <random>

namespace hof_tests {
using namespace hof;

inline void hof_test1() {
	const auto fn = []() { return true; };
	const auto b = logical_conjunction(fn, always(true));
	assert(b());
}

inline void hof_test2() {
	std::mt19937 engine;
	std::uniform_int_distribution<> dist(2, 3);
	const auto t = bind1st(dist, engine);
	auto u = bind1st(dist, engine);
	auto y = t();
	auto e = u();
}

inline void hof_test_tuple_fold() {
	auto a = []() { return std::make_tuple(1, 2, 3); };
	auto b = [](int,int,int) {
		return 2;
	};
	auto y = [](int) { return 3; };
	//auto d = [](std::tuple<int, int, int>) {return 2; };
	auto c = flow(a, b, y);
	c();

	auto e = []() { return std::make_tuple(1); };
	auto p = [](auto a) {
		using type = std::remove_cvref_t<decltype(a)>;
		static_assert(std::is_same_v<std::tuple<int>, type>);
	};
	auto w = flow(e, p);
	w();

	auto cw = compose(p, e);
	cw();


}
}
