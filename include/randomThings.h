#pragma once
#include <vector>
#include <deque>
#include <stack>
#include <list>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <utility>
#include <string>
#include <type_traits>
#include <fstream>
#include <array>
#include <forward_list>
#include <optional>
#include <experimental/coroutine>
#include <experimental/generator>
#include <atomic>
#include <functional>
#include <mutex>
#include <numeric>
//#include <ppl.h>
//#include <ppltasks.h>

//that might be usefull and not in the std already

template<typename T>
inline decltype(auto) derefCompletely(T& thing) {
	if constexpr(std::is_pointer_v<T>)
		return derefCompletely(*thing);
	else
		return thing;
}

template<typename fn>
struct Not{
	Not(fn t_f) : m_function(std::move(t_f)) {}
	template<typename ... Args>
	decltype(auto) operator()(Args... args) {
		return !m_function(std::forward<Args>(args)...);
	}
private:
	fn m_function;
};//callable that is returns !fn(args...) :D

template<typename T>
inline std::vector<T> Range(const T n) {
	std::vector<T> retVal(n);
	for (int i = 0; i < n; ++i)
		retVal[i] = i;	
	return retVal;
}

template<typename T>
inline std::vector<T> Range(T n1,const T n2) {
	std::vector<T> retVal;
	retVal.reserve(n2 - n1);
	for (; n1 < n2; ++n1) {		
		retVal.push_back(n1);
	}return retVal;
}

template<int size,class T>
inline constexpr std::array<T, size> RangeArray(T start) {
	std::array<T, size> retVal = {};
	for(int i = 0;i<size;++i)
		retVal[i] = start++;
	return retVal;
}

//I found this online
inline std::string getFileContents(const std::string& filePath,decltype(std::ios::in) mode = std::ios::in) {
	std::string fileContents;
	std::ifstream file(filePath, mode);
	file.seekg(0, std::ios::end);
	int filesize = (int)file.tellg();
	file.seekg(0, std::ios::beg);
	filesize -= (int)file.tellg();
	fileContents.resize(filesize);
	file.read(&fileContents[0], filesize);
	file.close();
	return fileContents;
}

constexpr int roundToNearest(const int num, const int multiply) {
	return (num / multiply)*multiply + multiply * !!(num%multiply);
}

template<int multiply>

constexpr int roundToNearestT(const int num) {
	return (num / multiply)*multiply + multiply * !!(num%multiply);
}

template<typename T>
struct getDimensions {
	static constexpr size_t value = 0;
};
template<typename T,size_t n>
struct getDimensions<std::array<T, n>> {
	static constexpr size_t value = getDimensions<T>::value + 1;
};
template<typename T>
struct getDimensions<std::vector<T>> {
	static constexpr size_t value = getDimensions<T>::value + 1;
};
template<typename T>
struct getDimensions<std::stack<T>> {
	static constexpr size_t value = getDimensions<T>::value + 1;
};
template<typename T,typename U>
struct getDimensions<std::unordered_map<T,U>> {
	static constexpr size_t value = getDimensions<U>::value + 1;
};
template<typename T,typename U>
struct getDimensions<std::map<T,U>> {
	static constexpr size_t value = getDimensions<U>::value + 1;
};
template<typename T>
struct getDimensions<std::list<T>> {
	static constexpr size_t value = getDimensions<T>::value + 1;
};
template<typename T>
struct getDimensions<std::forward_list<T>> {
	static constexpr size_t value = getDimensions<T>::value + 1;
};
template<typename T>
struct getDimensions<std::deque<T>> {
	static constexpr size_t value = getDimensions<T>::value + 1;
};

template<typename T1, typename T2, size_t size>
inline std::array<T1,size> array_cast(const std::array<T2,size>& oldArray){
	std::array<T1, size> retVal = {};
	for(size_t i = 0;i<size;++i)
		retVal[i] = static_cast<T1>(oldArray[i]);
	return retVal;
}

template<typename T1, typename T2, size_t size>
constexpr inline std::array<T1, size> array_cast2(const std::array<T2, size>& oldArray) {
	return array_cast_impl<T1>(oldArray, std::make_index_sequence<size>());
}
template<typename T1, typename T2, size_t size, size_t... i>
constexpr inline std::array<T1, size> array_cast_impl(const std::array<T2, size>& oldArray, std::index_sequence<i...>) {
	return { static_cast<T1>(oldArray[i])... };
}

inline bool isLetter(const char let) {
	return (let >= 'a' && let <= 'z' )|| (let >= 'A' && let <= 'Z');
}


template<typename string_t>
inline std::vector<string_t> split(string_t&& string, char letter = ' ') {
	std::vector<string_t> retVal;
	//retVal.reserve(3);
	size_t currentA = 0;
	do {
		size_t spot = string.find(letter, currentA);
		if (spot != currentA)
			retVal.push_back(string.substr(currentA, spot - currentA));
		currentA = spot + 1;
	} while (currentA);//std::string::npos + 1 = 0
	return retVal;
}

inline bool isNumber(char let){
	return let >= '0'&&let <= '9';
}

constexpr size_t out_of_range = -1;
template<typename string_t, typename... pred>
inline size_t find_first_if_not(string_t&& str, size_t off, pred... fn) {
	for (; off < str.size(); ++off){
		bool t = false;
		(void)(std::initializer_list<int>{(t |= fn(str[off]),0)...});
		//if (!(fn(str[off])||...)){}
		if(!t)
			return off;
	}return -1;
}

template<typename string_t, typename... pred>
inline size_t find_first_if(string_t&& str, size_t off, pred... fn) {
	for (; off < str.size(); ++off) {
		bool t = false;
		(void)(std::initializer_list<int>{(t |= fn(str[off]), 0)...});
		//if ((fn(str[off])||...))
		if (t)
			return off;
	}return -1;
}


template<typename... fns>
struct fnOr {
	constexpr fnOr(fns... functions) :m_fns(std::make_tuple(std::forward<fns>(functions)...)) {}
	template<typename... Args>
	decltype(auto) operator()(Args... args){
		return operator_impl(std::forward_as_tuple(std::forward<Args>(args)...), std::index_sequence_for<fns...>()); 
	}
private:
	std::tuple<fns...> m_fns;
	template<typename tuple,size_t... i>
	decltype(auto) operator_impl(tuple args,std::index_sequence<i...>){		
		return ((std::apply(std::get<i>(m_fns), args) || ...));
	}
};

template<typename... fns>
inline constexpr auto make_fnOr(fns... fn){
	return fnOr<fns...>(std::forward<fns>(fn)...);
}

template<typename T,typename adjFn, typename predFn>
inline T breadthFirstSearch(T start,adjFn adj, predFn pred){
	std::vector<T> queue(1, start);
	while (queue.size()){
		std::vector<T> next;
		for(T& node:queue){
			if(pred(node))
				return node;
			for(T&& newNode:adj(node))
				next.push_back(newNode);			
		}queue = std::move(next);
	}return {};
}

template<typename T, typename adjFn, typename predFn>
inline T breadthFirstSearchMultiple(T start, adjFn adj, predFn pred) {
	std::vector<T> queue(1, start);
	std::vector<T> retVal;
	while (queue.size()) {
		std::vector<T> next;
		next.reserve(queue.size());
		for (T& node : queue) {
			for (T& newNode : adj(node))
				next.push_back(std::move(newNode));
			if (pred(node))
				retVal.push_back(std::move(node));
		}queue = std::move(next);
	}return retVal;
}

template<typename T,typename adjFn,typename predFn>
inline std::pair<T,bool> depthFirstSearch(T start,adjFn adj,predFn pred){
	if (pred(start))
		return { start,true };
	for(auto&& next:adj(start)){
		const auto temp = depthFirstSearch(next, adj, pred);
		if(temp.second)
			return temp;		
	}return { start,false };
}

inline constexpr int ceilDiv(const int top, const int bot) {
	return (top / bot) + !!(top%bot);
}

inline std::vector<int> splitNumber(const int total,const int n){//splits a number into n ints, as equal as possible
	std::vector<int> retVal(n,total/n);
	for (int i = 0; i < total%n; ++i)
		++retVal[i];	
	return retVal;
}

inline std::experimental::generator<int> splitNumber_g(const int total, const int n) {//splits a number into "equal" ints
	for (int i = 0; i < total%n; ++i)
		co_yield (total / n) + 1;
	for (int i = total%n; i < n; ++i)
		co_yield total / n;
}
/*
template<typename T, typename adjFn, typename predFn>
inline T breadthFirstSearch_p(T start, adjFn adj, predFn pred) {
	std::vector<T> queue(1, start);
	std::optional<T> retVal;
	std::mutex mutex;
	std::vector<T> next;
	while (queue.size()&&!retVal.has_value()) {
		concurrency::parallel_for_each(queue.begin(),queue.end(),[&](T& item){
			if (pred(item))
				retVal = std::move(item);
			else{
				std::unique_lock<std::mutex> lock{ mutex };
				for(auto&& nextItem:adj(item))
					next.emplace_back(std::move(nextItem));				
			}
		});
		//queue = std::move(next);
		std::swap(queue, next);
		next.clear();
	}return retVal.value();
}
*/
template<typename T>
class circleInt {//IDK WHAT THIS IS USEFULL FOR D: ;-;
public:
	circleInt(T t_per):m_per(t_per){};
	circleInt& operator=(T thingy){
		m_stuff = std::move(thingy) % m_perimater;		
		return *this;
	}
	operator T(){
		return m_stuff;
	}
	circleInt& operator++(int){
		m_stuff = (m_stuff + 1)% m_perimater;
		return *this;
	}circleInt& operator++() {
		m_stuff = (m_stuff + 1) % m_perimater;
		return *this;
	}
	circleInt& operator+(T other){
		m_stuff += other;
		m_stuff = m_stuff%m_perimater;
		return *this;
	}
	circleInt& operator+=(T other){
		m_stuff += other;
		m_stuff = m_stuff % m_perimater;
		return *this;
	}
private:
	T m_stuff;
	const T m_perimater;
};

template<typename Container_t,typename pred>
inline Container_t& remove_if_all_quick(Container_t& vec,pred fn){
	//removes things faster :D, loses order
	vec.erase(std::partition(vec.begin(), vec.end(), not(fn)), vec.end());
	return vec;
}

template<typename container_t,typename pred>
inline void remove_if_quick(container_t& thing,pred fn) {
	std::swap(*std::find_if(thing.begin(), thing.end(), fn), thing.back());
	thing.pop_back();
}

template<typename T>
class view {//like std::string_view but for vectors!
public:
	view() = default;
	view(T* t_data,size_t t_size):m_data(t_data),m_size(t_size){}
	view(const std::vector<T>& vec):m_data(vec.data()),m_size(vec.size()){};
	const T& operator[](size_t index)const {
		return m_data[index];
	}
	void cut_right(const size_t len){
		m_size -= len;
	}
	void cut_left(const size_t len){
		m_data += len;
		m_size -= len;
	}
private:
	const T* m_data;
	size_t m_size = 0;
};

inline int strCount(std::string_view str, std::string_view thing) {
	int retVal = 0;
	while (str.size()>thing.size()) {
		if (str.substr(thing.size()) == thing) {
			++retVal;
			str.remove_prefix(thing.size());
		}
		else {
			const auto next = str.find(thing[0], 1);
			if (next == std::string_view::npos)//done
				return retVal;
			str.remove_prefix(next);
		}
	}return retVal;
}

template<typename T, typename adjFn, typename fn>
inline void BFSApply(T start, adjFn adj, fn Fn) {
	std::vector<T> queue(1, start);	
	while (queue.size()) {		
		//queue = std::move(queue) | transform([&](auto& i){std::invoke(adj,i);return std::invoke(adj,i);}) | join 
		std::vector<T> next;
		for (T& node : queue) {
			for (T& newNode : std::invoke(adj, node))
				next.push_back(newNode);
			Fn(node);
		}queue = std::move(next);
	}
}

enum class timerType {
	UP,
	DOWN
};

template<typename time_t,timerType type>
class async_timer {
public:
	async_timer() = default;
	async_timer(time_t t_timey,time_t t_interval = time_t(1)):
		m_timey(t_timey),
		m_interval(t_interval){}
	~async_timer(){
		stop();
	}

	async_timer& operator=(const async_timer&) = delete;
	async_timer& operator=(async_timer&& other)noexcept {
		m_timey = other.m_timey;
		m_interval = other.m_timey;
		m_running = other.m_running;
		m_thready = std::move(other.m_thready);
		return *this;
	};
	async_timer(const async_timer&) = delete;
	async_timer(async_timer&& other)noexcept {
		m_timey = other.m_timey;
		m_interval = other.m_timey;
		m_running = other.m_running;
		m_thready = std::move(other.m_thready);
	};

	void resume(){
		m_running = true;
		m_thready = std::thread([&, this]() {run(); });
	};
	void stop(){
		m_running = false;
		if (m_thready.joinable())
			m_thready.join();
	};
	time_t getTime()const { return m_timey; }
	void setTime(time_t t_timey){
		m_timey = std::move(t_timey);
	}
private:
	time_t m_timey;
	time_t m_interval = time_t(1);
	std::atomic<bool> m_running = false;
	std::thread m_thready;
	void run() {
		while (m_running && m_timey.count()>0) {
			std::this_thread::sleep_for(m_interval);
			if constexpr(type==timerType::DOWN){
				--m_timey;
			}else{
				++m_timey;
			}
		}m_running = false;
	};
}; 

template <typename T>
struct crtp
{
	T& self() { return static_cast<T&>(*this); }
	T const& self() const { return static_cast<T const&>(*this); }	
};


template<typename T,bool b>
struct addRefIf {
	using type = T;
};

template<typename T>
struct addRefIf<T, true> {
	using type = std::add_lvalue_reference_t<T>;
};

template<typename T,bool b>
using addRefIf_t = typename addRefIf<T, b>::type;

struct NOT {
	template<typename T>
	constexpr decltype(auto) operator()(T input) {
		return !input;
	}
};

template<typename f1,typename ... fns>
struct propagate_fn {
	template<typename ...Args>
	constexpr decltype(auto) operator()(Args... args)const{	
		if constexpr(sizeof...(fns)!=0){
			return propagate_fn<fns...>()(f1()(std::forward<Args>(args)...));
		}else{
			return f1()(std::forward<Args>(args)...);
		}
	}	
};

template<typename comp,typename T>
struct comparator {
	constexpr comparator(T t_n):m_n(std::move(t_n)){}

	template<typename U>
	constexpr bool operator()(const U& other)const{
		return comp{}(other,m_n);
	}
	const T m_n;
};

//template<typename T,typename comp>
//comparator(T)->comparator<T, comp>;

template<typename T>
using greaterThan = comparator<std::greater<T>, T>;

template<typename T>
using lessThan = comparator<std::less<T>,T>;

template<typename T>
using equalTo = comparator<std::equal_to<T>,T>;

template<typename T>
using notDivisibleBy = comparator<std::modulus<T>, T>;

template<typename T>
using divisibleBy = comparator<propagate_fn<std::modulus<T>, NOT>,T>;

template<typename range>
std::experimental::generator<std::pair<size_t, decltype(*(std::declval<range>().begin()))>> enumerate(range&& r){
	size_t i = 0;
	for(decltype(auto) item:r)
		co_yield{ i++, item};	
}

template<typename Range>
struct enumerate2{
	using iterator_type = std::decay_t<decltype(std::declval<Range>().begin())>;
	using end_iterator_type = std::decay_t<decltype(std::declval<Range>().end())>;
	explicit enumerate2(Range& r):m_current(r.begin()),m_end(r.end()){} 

	decltype(auto) begin(){
		return *this;
	}
	std::pair<decltype(*(std::declval<Range>().begin())), size_t> operator*(){
		return { *m_current,i };
	}
	decltype(auto) operator++(){
		++m_current;
		++i;
		return *this;
	}
	decltype(auto) operator++(int) {
		++m_current;
		++i;
		return *this;
	}
	struct endy{};
	endy end(){
		return endy();
	}

	bool operator!=(endy){
		return m_current != m_end;
	}
private:
	iterator_type m_current;
	const end_iterator_type m_end;
	size_t i = 0;
};


class sync_timer {
public:
	sync_timer() = default;
	
	void start(){
		if(m_starts.size() == m_ends.size())
			m_starts.push_back(std::chrono::system_clock::now());
	}
	void stop(){
		if (m_starts.size() == m_ends.size() + 1)
			m_ends.push_back(std::chrono::system_clock::now());		
	}
	void reset(){
		m_starts.clear();
		m_ends.clear();
	}
	template<typename time_t>
	time_t getTime()const{
		return
			(m_starts.size() > m_ends.size() ? std::chrono::duration_cast<time_t>(std::chrono::system_clock::now() - m_starts.back()) : time_t(0)) + 
			std::inner_product(m_ends.begin(), m_ends.end(), m_starts.begin(), time_t(0), std::plus<>(), [](auto a, auto b){
				return std::chrono::duration_cast<time_t>(a - b);
			});
	}
private:
	std::vector<std::chrono::system_clock::time_point> m_starts;
	std::vector<std::chrono::system_clock::time_point> m_ends;
};


template<typename T>
inline void reorder(std::vector<T>& vec, const std::vector<int>& newOrder) {
	//reorder a vector based on indexes
	std::vector<T> newVec(newOrder.size());
	for (int i = 0; i<vec.size(); ++i)
		newVec[newOrder[i]] = std::move(vec[i]);
	vec = std::move(newVec);
}

template<typename T>
inline void unorder(std::vector<T>& vec, const std::vector<int>& newOrder) {
	//reorder a vector based on indexes
	std::vector<T> newVec(newOrder.size());
	for (int i = 0; i<vec.size(); ++i)
		newVec[i] = std::move(vec[newOrder[i]]);
	vec = std::move(newVec);
}

template<typename T>
inline std::vector<T> reorder2(const std::vector<T>& vec, const std::vector<int>& newOrder) {
	//reorder a vector based on indexes
	std::vector<T> newVec(newOrder.size());
	for (int i = 0; i<vec.size(); ++i)
		newVec[newOrder[i]] = vec[i];
	return newVec;
}

template<typename T>
struct reorder_vector_iterator {//bad name ;-;
	reorder_vector_iterator(std::vector<T>& vec, std::vector<int> t_indices) :m_vec(vec), m_indices(t_indices) {

	};
	decltype(auto) begin() {
		return *this;
	}

	struct endy {};

	static endy end() {
		return {};
	}

	T& operator*() {
		return m_vec[m_current];
	}

	decltype(auto) operator++() {
		m_current++;
		return *this;
	}

	decltype(auto) operator++(int) {
		m_current++;
		return *this;
	}

	bool operator!=(endy) {
		return m_current < m_indices.size();
	}
private:
	std::vector<T>& m_vec;
	std::vector<int> m_indices;
	int m_current = 0;
};

inline unsigned swapEndianness(unsigned input) {
	return input << 24 | input >> 24 | ((input << 8) & (0xFF << 16)) | ((input >> 8) & (0xFF << 8));
}


static constexpr uint64_t byte0 = 0xFF;
static constexpr uint64_t byte1 = 0xFF00;
static constexpr uint64_t byte2 = 0xFF0000;
static constexpr uint64_t byte3 = 0xFF000000;
static constexpr uint64_t byte4 = 0xFF00000000;
static constexpr uint64_t byte5 = 0xFF0000000000;
static constexpr uint64_t byte6 = 0xFF000000000000;
static constexpr uint64_t byte7 = 0xFF00000000000000;


template<typename T>
inline uint64_t readBit(const T* v, const size_t n) {
	return readBit((uint8_t*)v, n);
}

inline uint64_t readBit(const uint8_t* v, const size_t n) {
	return ((v[n >> 3]) >> ((n & 7))) & 0b1;
}

template<typename T>
inline uint64_t readBit(const std::vector<T>& v, size_t n) {
	return readBit((uint8_t*)v.data(), n);
}
template<typename T>
inline size_t readBits(const T* v, size_t start, const size_t end) {
	size_t retVal = 0;
	for (int bit = 0; start < end; ++bit)
		retVal += readBit(v, start++) << bit;
	return retVal;
}

template<typename T>
inline size_t readBits(const std::vector<T>& v, const size_t start, const size_t end) {
	return readBits(v.data(), start, end);
}

template<typename T>
inline T round_to_multiple(const T in, const T m) {
	return ((in / m) + !!(in % m)) * m;
}

template<typename T>
inline size_t readBitsReversed(const T* v, size_t start, const size_t end) {
	size_t retVal = 0;
	for (int bit = 0; start < end; ++bit)
		retVal = (retVal << 1) + readBit(v, start++);
	return retVal;
}
template<typename T>
inline size_t readBitsReversed(const std::vector<T>& v, size_t start, const size_t end) {
	return readBitsReversed(v.data(), start, end);
}

template<typename it>
class strideIterator {
public:
	strideIterator() = delete;
	strideIterator(it t_iterator, const size_t t_stride) :m_iterator(t_iterator), m_stride(t_stride) {};

	decltype(auto) operator*() {
		return *m_iterator;
	}

	decltype(auto) operator++() {
		std::advance(m_iterator, m_stride);
		return *this;
	}
	decltype(auto) operator++(int) {
		std::advance(m_iterator, m_stride);
		return *this;
	}
	bool operator!=(const it other) {
		return m_iterator < other;
	}
	bool operator!=(const strideIterator<it> other) {
		return m_iterator < other.m_iterator;
	}
	bool operator<(const it other) {
		return m_iterator < other;
	}
	bool operator<(const strideIterator<it> other) {
		return m_iterator < other.m_iterator;
	}

private:
	it m_iterator;
	const size_t m_stride;
};


/*
template<typename it>
strideIterator(it, size_t)->strideIterator<it>;
*/

template<typename T>
class copy_on_write {
	struct ref_t {
		ref_t() = default;
		ref_t(T t_self) :self(std::move(t_self)) {};
		T self;
		size_t ref_count = 1;
	};
public:
	copy_on_write() = default;
	copy_on_write(T thing):m_self(new ref_t(std::move(thing))) {
	};
	~copy_on_write() {
		if (!m_self) return;
		--m_self->ref_count;
		if (!m_self->ref_count)
			delete m_self;
	}

	copy_on_write(const copy_on_write& other) {
		~copy_on_write();
		m_self = other.m_self;
		++m_self->ref_count;
	}
	copy_on_write(copy_on_write&& other)noexcept {
		~copy_on_write();
		std::swap(other.m_self, m_self);
	}
	copy_on_write& operator=(const copy_on_write& other) {
		~copy_on_write();
		m_self = other.m_self;
		++m_self->ref_count;
		return *this;
	}
private:
	ref_t * m_self = new ref_t;
};


inline unsigned reverseBits(const unsigned stuffs, const int numBits) {
	unsigned retVal = 0;
	for (int i = 0; i<numBits; ++i) {
		retVal |= ((stuffs >> i) & 1) << (numBits - i - 1);
	}return retVal;
}



template<typename other>
class fake_iterator {
public:
	other & begin() {
		return *this;
	}
	other& end() {
		return *this;
	}

	other& operator++() {
		static_cast<other*>(this)->increment();
		return *this;
	}
	other& operator++(int) {
		static_cast<other*>(this)->increment();
		return *this;
	}
	bool operator!=(other Other) {
		return static_cast<other*>(this)->endCondition(Other);
	}
	bool operator<(other Other) {
		return static_cast<other*>(this)->endCondition(Other);
	}
};


template<typename other>
class fake_sentinel_iterator {
public:
	struct sentinel {};
	other& begin() {
		return *this;
	}
	sentinel end() {
		return {};
	}
	other& operator++() {
		static_cast<other*>(this)->increment();
		return *this;
	}
	other& operator++(int) {
		static_cast<other*>(this)->increment();
		return *this;
	}
	bool operator!=(sentinel Other) {
		return static_cast<other*>(this)->endCondition();
	}
	bool operator<(sentinel Other) {
		return static_cast<other*>(this)->endCondition();
	}

private:

};

template<typename T>
struct generatorOrVoid {
	using type = std::experimental::generator<T>;
};

template<>
struct generatorOrVoid<void>{
	using type = void;
};


template<typename T>
class delegate;

template<typename retType, typename... Args>
class delegate<retType(Args...)>{
public:
	typename generatorOrVoid<retType>::type operator()(Args... args)const{
		for(auto& i:m_stuffs){
			if constexpr(!std::is_same_v<retType, void>)
				co_yield i(std::forward<Args>(args)...);
			else
				i(std::forward<Args>(args)...);
		}
	}
	void add(std::function<retType(Args...)> thing) {		
		m_stuffs.push_back(std::move(thing));
	}

private:
	std::vector<std::function<retType(Args...)>> m_stuffs;
};

struct genericComp {
	using is_transparent = int;
	template<typename T,typename T2>
	bool operator()(const T& a,const T2& b){
		return a < b;
	}
};


namespace zipy{
template<typename Tuple,size_t... i>
void iterateAll(Tuple& tuple,std::index_sequence<i...>) {
	((++std::get<i>(tuple), ...));
}

template<typename Tuple,typename Tuple2, size_t... i>
bool reachedEnd(Tuple& starts, const Tuple2& ends,std::index_sequence<i...>) {
	return (((std::get<i>(tuple) != std::get<i>(ends)) || ...));
}

template<typename outTuple, typename inTuple, size_t...i >
outTuple dereferenceIn(inTuple& in,std::index_sequence<i...>) {
	return outTuple{ *std::get<i>(in)... };
}
}

template<typename ... ranges>
std::experimental::generator<std::tuple<decltype(*std::declval<ranges>().begin())... >> zip(ranges... Ranges) {
	using tuple_type = std::tuple<decltype(*std::begin(std::declval<ranges>()))... >;
	const auto indexThing = std::index_sequence_for<ranges...>();

	std::tuple<decltype(std::declval<ranges>().begin())... > iterators	= { std::begin(Ranges)... };
	const std::tuple<decltype(std::declval<ranges>().end())... > ends	= { std::end(Ranges)... };

	while(!zipy::reachedEnd(iterators,ends, indexThing)) {
		co_yield zipy::dereferenceIn<tuple_type>(iterators, indexThing);
		zipy::iterateAll(iterators, indexThing);
	}
}


template<typename T1,typename T2>
std::vector<T1> vector_cast(std::vector<T2>&& other) {
	std::vector<T1> retVal(other.size());
	for (size_t i = 0; i < other.size(); ++i)
		retVal[i] = static_cast<T1>(other[i]);
	return retVal;
}

inline std::string_view removeEnds(const std::string& in) {
	std::string_view out(in);
	out.remove_prefix(1);
	out.remove_suffix(1);
	return out;
}

inline std::string_view removeEnds(std::string&& in) {
	std::string_view out(in);
	out.remove_prefix(1);
	out.remove_suffix(1);
	return out;
}


template<typename T>
static inline const T default_val = std::enable_if_t<std::is_default_constructible_v<T>, T>{};

template<size_t n,typename T,typename ...Ts>
struct nth_type{
	using type = nth_type<n - 1, Ts...>;
};

template<typename T,typename... Ts>
struct nth_type<0,T,Ts...>{
	using type = T;
};


template<typename T, typename ... Ts,size_t... i>
T construct_from_tuple_impl(std::tuple<Ts...>&& tup,std::index_sequence<i...>) {
	return T(std::get<i>(tup));
}

template<typename T,typename ... Ts>
T construct_from_tuple(std::tuple<Ts...>&& tup) {
	return construct_from_tuple_impl(std::move(tup));
}

template<typename T, typename ... Ts, size_t... i>
T construct_from_tuple_impl(const std::tuple<Ts...>& tup, std::index_sequence<i...>) {
	return T(std::get<i>(tup));
}

template<typename T, typename ... Ts>
T construct_from_tuple(const std::tuple<Ts...>& tup) {
	return construct_from_tuple_impl(tup);
}
