#pragma once
#include <memory>
#include <utility>
#include <tuple>
#include <vector>
namespace hof {
	template<typename T, typename comp>
	struct comparator {
		constexpr explicit comparator(T a) :value(std::move(a)) {}

		constexpr explicit comparator(T a, comp b) :value(std::move(a)), c(std::move(b)) {}

		template<typename U>
		constexpr bool operator()(U&& other) {
			return std::invoke(c, value, std::forward<U>(other));
		}

		T value = {};
		comp c = {};
	};

	template<typename T>
	struct is_less_than {
		template<typename TT, std::enable_if_t<!std::is_same_v<std::decay_t<TT>, is_less_than<TT>>, int> = 0>
		constexpr explicit is_less_than(TT&& a) :value(std::forward<TT>(a)) {}

		template<typename U>
		constexpr bool operator()(U&& other) {
			return other < value;
		}
		T value;
	};

	template<typename TT>
	is_less_than(TT&&)->is_less_than<TT>;

	template<typename T>
	struct is_greater_than {

		template<typename TT,std::enable_if_t<!std::is_same_v<std::decay_t<TT>,is_greater_than<TT>>,int> = 0>
		constexpr explicit is_greater_than(TT&& a) :value(std::forward<TT>(a)) {}

		template<typename U>
		constexpr bool operator()(U&& other) {
			return other > value;
		}
		T value;
	};

	template<typename TT>
	is_greater_than(TT&&)->is_greater_than<TT>;

	template<typename T>
	struct is_less_than_eq {
		template<typename TT, std::enable_if_t<!std::is_same_v<std::decay_t<TT>, is_less_than_eq<TT>>, int> = 0>
		constexpr explicit is_less_than_eq(TT&& a) :value(std::forward<TT>(a)) {}

		template<typename U>
		constexpr bool operator()(U&& other) {
			return other <= value;
		}
		T value;
	};

	template<typename TT>
	is_less_than_eq(TT&&)->is_less_than_eq<TT>;

	template<typename T>
	struct is_greater_than_eq {
		template<typename TT, std::enable_if_t<!std::is_same_v<std::decay_t<TT>, is_greater_than_eq<TT>>, int> = 0>
		constexpr explicit is_greater_than_eq(TT&& a) :value(std::forward<TT>(a)) {}

		template<typename U>
		constexpr bool operator()(U&& other) {
			return other >= value;
		}
		T value;
	};

	template<typename TT>
	is_greater_than_eq(TT&&)->is_greater_than_eq<TT>;

	template<typename T>
	struct is_equal_to {
		template<typename TT, std::enable_if_t<!std::is_same_v<std::decay_t<TT>, is_equal_to<TT>>, int> = 0>
		constexpr explicit is_equal_to(TT&& a) :value(std::forward<TT>(a)) {}

		template<typename U>
		constexpr bool operator()(U&& other) {
			return other == value;
		}
		T value;
	};

	template<typename TT>
	is_equal_to(TT&&)->is_equal_to<TT>;

	template<typename T>
	struct is_not_equal_to {
		template<typename TT, std::enable_if_t<!std::is_same_v<std::decay_t<TT>, is_not_equal_to<TT>>, int> = 0>
		constexpr explicit is_not_equal_to(TT&& a) :value(std::forward<TT>(a)) {}

		template<typename U>
		constexpr bool operator()(U&& other) {
			return other != value;
		}
		T value;
	};

	template<typename TT>
	is_not_equal_to(TT&&)->is_not_equal_to<TT>;

	//f1(f2(f3(args...)))
	template<typename First, typename ...Rest>
	struct compose :compose<Rest...> {
		constexpr compose() = default;

		template<typename F, typename... R>
		constexpr explicit compose(F&& f, R&& ... rest) :compose<Rest...>(std::forward<R>(rest)...), first_fn(std::forward<F>(f)) {}

		template<typename ...Args>
		constexpr decltype(auto) operator()(Args&& ... args) {
			//return static_cast<compose<Rest...>&>(*this)(std::invoke(first_fn, std::forward<Args>(args)...));
			return std::invoke(first_fn, static_cast<compose<Rest...>&>(*this)(std::forward<Args>(args)...));
		}

		First first_fn = {};
	};

	template<typename Last>
	struct compose<Last> {
		constexpr compose() = default;

		template<typename L,std::enable_if_t<!std::is_same_v<std::decay_t<L>,compose<L>>,int> = 0>
		constexpr explicit compose(L&& last) :last_fn(std::forward<L>(last)) {}

		template<typename... Args>
		constexpr decltype(auto) operator()(Args&& ... args) {
			return std::invoke(last_fn, std::forward<Args>(args)...);
		}

		Last last_fn = {};
	};

	template<typename F, typename... R>
	compose(F&&, R&&...)->compose<F, R...>;

	template<typename L>
	compose(L&&)->compose<L>;

	//f3(f2(f1(args...)))
	template<typename First, typename ...Rest>
	struct fold :fold<Rest...> {
		constexpr fold() = default;

		template<typename F, typename... R>
		constexpr explicit fold(F&& f, R&& ... rest) :fold<Rest...>(std::forward<R>(rest)...), first_fn(std::forward<F>(f)) {}

		template<typename ...Args>
		constexpr decltype(auto) operator()(Args&& ... args) {
			return static_cast<fold<Rest...>&>(*this)(std::invoke(first_fn, std::forward<Args>(args)...));
		}
		First first_fn = {};
	};

	template<typename Last>
	struct fold<Last> {
		constexpr fold() = default;

		template<typename L, std::enable_if_t<!std::is_same_v<std::decay_t<L>, fold<Last>>, int> = 0>
		constexpr explicit fold(L&& last) :last_fn(std::forward<L>(last)) {}

		template<typename... Args>
		constexpr decltype(auto) operator()(Args&& ... args) {
			return std::invoke(last_fn, std::forward<Args>(args)...);
		}
		Last last_fn = {};
	};

	template<typename F, typename... R>
	fold(F&&, R&&...)->fold<F, R...>;

	template<typename L>
	fold(L&&)->fold<L>;

	template<typename ...fns>
	struct logical_conjunction {
		static_assert(sizeof...(fns) >= 1);
		constexpr logical_conjunction() = default;
		constexpr explicit logical_conjunction(fns... funcs) :functions(std::move(funcs)...) {}

		template<typename ...Args>
		constexpr decltype(auto) operator()(Args&&... args) {
			if constexpr (sizeof...(fns) == 1) {
				return std::invoke(std::get<0>(functions), std::forward<Args>(args)...);
			} else {
				return std::apply([&](auto&&... funcs) {
					return (std::invoke(funcs, args) && ...);
				}, functions);
			}
		}

		std::tuple<fns...> functions = {};
	};

	template<typename ...fns>
	struct logical_disjunction {
		static_assert(sizeof...(fns) >= 1);
		constexpr logical_disjunction() = default;

		constexpr explicit logical_disjunction(fns... funcs) :functions(std::move(funcs)...) {}

		template<typename ...Args>
		constexpr decltype(auto) operator()(Args&&... args) {
			if constexpr (sizeof...(fns) == 1) {
				return std::invoke(std::get<0>(functions), std::forward<Args>(args)...);
			} else {
				return std::apply([&](auto&&... funcs) {
					return (std::invoke(funcs, args) || ...);
				}, functions);
			}
		}

		std::tuple<fns...> functions = {};
	};

	template<typename fn>
	struct logical_negate {
		constexpr logical_negate() = default;

		template<typename F,std::enable_if_t<!std::is_same_v<std::decay_t<F>,logical_negate<F>>,int> = 0>
		constexpr explicit logical_negate(F&& a) :func(std::forward<F>(a)) {}

		template<typename ...T>
		constexpr decltype(auto) operator()(T&&...a) {
			return !std::invoke(func, std::forward<T>(a)...);
		}
		fn func = {};
	};

	template<typename F>
	logical_negate(F&&)->logical_negate<F>;

	template<typename map_type>
	struct map_with {

		template<typename map_type_t>
		constexpr explicit map_with(map_type_t&& m) :map(std::forward<map_type_t>(m)) {}

		template<typename T>
		constexpr decltype(auto) operator()(T&& i) {
			return map[std::forward<T>(i)];
		}

		map_type map;
	};

	template<typename T>
	map_with(T&&)->map_with<T>;

	struct dereference {
		template<typename T>
		constexpr auto operator()(T&& t)const noexcept(noexcept(*t)) ->decltype(*t) {
			return *t;
		}
	};

	static constexpr auto always = [](auto&& a) {
		return[b = std::forward<decltype(a)>(a)](auto&&...)->auto&{
			return b;
		};
	};

	static constexpr auto flip_args = [](auto&& fn) {
		return[fn_ = std::forward<decltype(fn)>(fn)](auto&& a, auto&& b){
			return std::invoke(fn_, std::forward<decltype(b)>(b), std::forward<decltype(a)>(a));
		};
	};

	static constexpr auto square = [](auto&& a) {
		return a * a;
	};

	template<typename fn, typename Arg1>
	struct bind1st {
		template<typename F, typename A>
		constexpr bind1st(F&& f, A&& a) :func(std::forward<F>(f)), arg(std::forward<A>(a)) {}

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
		constexpr bind2nd(F&& f, A&& a) :func(std::forward<F>(f)), arg2(std::forward<A>(a)) {}

		template<typename Arg1>
		constexpr decltype(auto) operator()(Arg1&& arg) {
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
		constexpr bind_last(F&& f, A&& a) :fn(std::forward<F>(f)), arg(std::forward<A>(a)) {}

		template<typename... Args>
		auto operator()(Args&&... args) const->std::invoke_result_t<Fn, Args..., Argl> {
			return std::invoke(fn, std::forward<Args>(args)..., arg);
		}

		Fn fn;
		Argl arg;
	};

	template<typename F, typename A>
	bind_last(F&&, A&&)->bind_last<F, A>;

	inline void asdasdasd() {
		auto i = fold([]() {return 1; }, [](int u) {return 5; });
		auto t = i();
		auto qweds = compose([] (int){}, [](int a) {return 2; });
		qweds(2);
		auto fnsasd = logical_conjunction(i, [](int y) {return false; });
		std::vector<int> aaa;
		auto qwe = map_with(std::move(aaa));
	}

}