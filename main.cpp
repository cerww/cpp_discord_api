#include <iostream>
#include <boost/asio.hpp>
#include <utility>
#include <string>
#include <boost/beast.hpp>
#include <future>
//#include <uWS/uWS.h>
#include "client.h"
#include "stuffs.h"
#include "randomThings.h"
#include "indirect.h"
#include <memory_resource>

using namespace std::string_literals;
using namespace std::chrono_literals;
class u;
template<typename T>
struct wat{};

using q = wat<u>;
template<typename T>
struct po{};

class timer {
public:
	void start() {
		startTime = std::chrono::high_resolution_clock::now();
	}
	template<typename duration>
	duration timeSinceStart()const {
		return std::chrono::duration_cast<duration>((std::chrono::high_resolution_clock::now() - startTime));
	}
private:
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
};

template<typename T>
std::experimental::generator<std::vector<T>> permutations_bad(std::vector<T> in) {
	if (in.size() == 1) {
		co_yield in;
		co_return;
	}
	for (int i = 0; i<in.size(); ++i) {
		std::swap(in[i], in[0]);
		std::vector<T> wat(in.begin() + 1, in.end());
		for (std::vector<T> v : permutations_bad(wat)) {
			v.insert( v.begin(),in[0]);
			co_yield v;
		}
	}
}
template<typename it>
std::experimental::generator<int> permutations_bad(it s, it e) {
	if (std::distance(s,e) == 1) {
		co_yield 0;
		co_return;
	}
	for (auto o = s; o!=e; ++o) {
		std::iter_swap(s, o);		
		for (auto v : permutations_bad(s+1,e)) 
			co_yield 0;
		std::iter_swap(s, o);
		//swaps them back to original order if permutations_bad(s+1,e) doesnt change the order
		//permutations_bad(s+1,e) won't change any order if std::distance(s,e) == 2
		//so std::iter_swap(s, o) when there are 2 items will swap things back to the original order
		//and since it won't change the order of 2 items, it won't change the order of 3 items
	}
}
struct s{
	int qa;
	guild_member b;
	std::allocator<int> v;
};


template<typename T>
struct async_generator{	
	struct promise_type {
		my_concurrent_queue<T> queue;
		std::atomic<bool> done = false;
		std::experimental::suspend_always initial_suspend() const{
			return {};
		}
		std::experimental::suspend_always final_suspend(){	
			done.store(true);
			return {};
		}
		std::experimental::suspend_if yield_value(T t) {
			queue.push(std::move(t));
			return std::experimental::suspend_if{ done.load() };
		}
		auto get_return_object() {
			return async_generator<T>(std::experimental::coroutine_handle<promise_type>::from_promise(*this));
		}
		void return_void() const{}
		
	};

	async_generator(std::experimental::coroutine_handle<promise_type> coro):m_coro(coro) {
		m_thread = std::thread(&std::experimental::coroutine_handle<promise_type>::resume,m_coro);
	}
	async_generator(async_generator&& other)noexcept {
		m_coro = other.m_coro;
		other.m_coro = nullptr;
		std::swap(other.m_thread, m_thread);
	}
	~async_generator() {
		if(m_coro){
			m_coro.promise().done.store(true);
			if(m_thread.joinable())
				m_thread.join();
			m_coro.destroy();
		}else {
			if (m_thread.joinable())
				m_thread.join();
		}
	}
	struct done_iter {};
	static constexpr inline done_iter sentinal = done_iter{};
	struct iterator{
		iterator(async_generator& t) :m_parent(t) {}
		iterator& operator++()    { return *this; }
		iterator& operator++(int) { return *this; }
		T operator*() {
			return m_parent.m_coro.promise().queue.pop();
		}
		bool operator!=(done_iter) { return m_parent.m_coro.promise().queue.size() || !m_parent.m_coro.promise().done.load(); }
	private:
		async_generator& m_parent;
	};
	auto begin() {
		return iterator(*this);
	}
	auto end() { return sentinal; }
	
private:
	std::experimental::coroutine_handle<promise_type> m_coro = nullptr;
	std::thread m_thread;
	friend struct iterator;
};


/*
template<typename R,typename ... Args>
struct std::experimental::coroutine_traits<async_generator<R>,Args...>{
	
};
*/

async_generator<int> test1() {
	co_yield 1;
	co_yield 2;
	co_yield 1;
	co_yield 2;
	co_yield 1;
	co_yield 2;
	co_yield 1;
	co_yield 2;
	co_yield 1;
	co_yield 2;
	co_yield 1;
	co_yield 2;
	co_yield 1;
	co_yield 2;
	co_yield 1;
	co_yield 1;
	co_yield 2;
	co_yield 1;
	co_yield 2;
	co_yield 1;
	co_yield 2;
	co_yield 2;
}
template<typename event_type>
struct subscriber_thingy_impl{
	struct sentinal {};
	struct awaitable_iterator {
		awaitable_iterator(subscriber_thingy_impl& t) :parent(t) {}
		bool await_ready() {
			return false;
		}
		void await_suspend(std::experimental::coroutine_handle<> h) {
			parent.stuff.push({ h,&eventu });
		}
		awaitable_iterator await_resume() {
			return *this;
		}
		awaitable_iterator& operator++() { return *this; }
		awaitable_iterator& operator++(int) { return *this; }
		bool operator!=(sentinal)const { return true; }
		event_type& operator*() {
			return eventu;
		}
	private:
		event_type eventu;
		subscriber_thingy_impl& parent;
		friend subscriber_thingy_impl;
	};
	auto begin() {
		return awaitable_iterator{ *this };
	}
	auto end() { return sentinal{}; }
protected:
	my_concurrent_queue<std::pair<std::experimental::coroutine_handle<>, event_type*>> stuff;
};

template<typename event_type>
struct subscriber_thingy:subscriber_thingy_impl<event_type>{
	[[nodiscard]]
	std::future<void> add_event(event_type e){		
		return std::async(std::launch::async, [_e = std::move(e), this]()mutable {auto t = stuff.pop(); *t.second = std::move(_e); t.first.resume(); });
	}
};

template<typename event_type>
struct subscriber_thingy_no_async :subscriber_thingy_impl<event_type> {	
	void add_event(event_type e) {
		auto t = stuff.pop();
		*t.second = std::move(e);
		t.first.resume();
	}
};

std::future<void> blargu(subscriber_thingy_no_async<std::string>& s) {
	for co_await(auto&& waty:s) {
		std::this_thread::sleep_for(1s);
		std::cout << waty << std::endl;
	}
}

struct no_suspend{
	std::experimental::suspend_never initial_suspend() { return {}; }
	std::experimental::suspend_never final_suspend() { return {}; }
};

struct thread_coro{
	struct promise_type:no_suspend{
		auto get_return_object() {
			return std::experimental::coroutine_handle<promise_type>::from_promise(*this);
		}
		std::thread worker;
	};
};

thread_coro fun2(subscriber_thingy<std::function<void()>>& s) {
	for co_await(auto&& f:s) {
		
	}
}

struct thread_pooly{
	thread_pooly() {
		
	}

	~thread_pooly() {
		m_done.store(true);
		for (auto& t : m_thread)t.join();
	}

private:
	struct coro_thingy{
		struct promise_type:no_suspend{
			my_concurrent_queue<std::function<void()>> q;

			coro_thingy get_return_value();
				
			

		};
		std::experimental::coroutine_handle<promise_type> hand;
	};
	std::future<void> coro_run() {
		for co_await(auto&& f:m_thing) {
			f();
		}
	}
	void run() {
		auto t = coro_run();
		
	}
	
	std::atomic<bool> m_done = false;
	std::vector<std::thread> m_thread;
	subscriber_thingy<std::function<void()>> m_thing;
};

thread_pooly::coro_thingy thread_pooly::coro_thingy::promise_type::get_return_value() {
	coro_thingy c;
	c.hand = decltype(c.hand)::from_promise(*this);
	return c;
}

void braweadsasde() {
	std::string str = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.";
	//std::cin >> str;
	std::array<char, 500> data = {};
	size_t ptr = 0;
	std::vector<size_t> loop_starts;
	for (size_t i = 0; i < str.size(); ++i) {		
		if (str[i] == '<') {
			ptr--;
		}
		else if (str[i] == '>') {
			ptr++;
		}
		else if (str[i] == '+') {
			++data[ptr];
		}
		else if (str[i] == '-') {
			--data[ptr];
		}
		else if (str[i] == '[') {
			loop_starts.push_back(i);
		}
		else if (str[i] == ']') {
			if (!data[ptr]) {
				loop_starts.pop_back();
			}
			else {
				i = loop_starts.back();
			}
		}
		else if (str[i] == '.') {
			std::cout << data[ptr];
		}
		else if (str[i] == ',') {
			std::cin >> data[ptr];
		}
	}
	std::cin.get();
}

template<typename it,typename binaryPred>
it unique_l(it s,it e,binaryPred f) {
	while (s < e){
		const it o = std::find_if_not(s, e, [&](auto&& a) {return f(a, *s); });
		e = std::rotate(++s, o, e);
	}return e;
}

template<typename it>
it unique_l(it s, it e) {
	return unique_l(s, e, std::equal_to<>());
}

int main(){
	try{
		client c;
		std::vector<partial_message> msgs;
		c.on_guild_text_msg = [&](guild_text_message& wat,shard& s){
			if(wat.content() == "watland") {
				s.delete_message(msgs.back()).get();
				msgs.pop_back();
			}
			else if(wat.content()== "make new channel") {
				s.create_text_channel(wat.guild(),"blargy").get();
			}
			//s->change_nick(wat.guild(),wat.author(), wat.content());
			for(auto& i:wat.mentions()){
				s.change_nick(wat.guild(), *i, wat.content());
			}
			if (wat.author().id() != c.getSelf().id())
				s.send_message(wat.channel(), std::to_string(wat.author().id().val));
			s.add_reaction(wat,wat.guild().emojis().back());
		};
		c.on_guild_typing_start = [&](guild_member& member,text_channel& channel,shard& s){
			try{
				msgs.push_back(s.send_message(channel,member.username()+ " has started typing").get());
			}catch(...){}
		};
		c.on_guild_member_add = [&](guild_member& member,shard& s){
			s.send_message(member.guild().general_channel(),member.username()).get();
		};
		c.setToken(tokenType::BOT, "NDAxNjE3Njc4MDk3MTg2ODE3.DWfIcA.EkhHmYUcv3iA4w4IMlfIo1KWysY"s);
		c.run();
	}catch(...) {
		std::cout << ";-;" << std::endl;
	}
	std::cin.get();
}



/*
std::unordered_map<std::string,int> grades = f | split('\n') | transform([](auto&& str){return str|split(' ');}) | transform([](auto&& stuff)->std::pair<std::string,int> {
	return {std::move(stuff|first),std::inner_product(stuff|tail,correct,std::equal_to<>()};
}); 
std::vector<std::vector<int>> o(35,std::vector<int>(10));
for(int i = 0;i<stuff.size();++i){
	t = split(stuff[i]);
	for(int u = 1;u<t.size();++u)
		o[i][u] = t[u];
}

vec correct ;
//cin
std::vector<int> wat(35,0);
for(int i = 0;i<wat.size();++i){
	for(int u = 0;u<o[i].size();++u){
		wat[i] += o[i][u] == correct[u];
	}
}

*/
/*
#include <math.h>
#include <stdio.h>

#define WHITE 1
#define BLACK 2

#define NONE 0
#define PAWN 4
#define BISH 8
#define ROOK 16
#define KING 32
#define KNIGHT 64
#define QUEEN 128

int dv(int m_sslCtx) {
	if (m_sslCtx)
		return m_sslCtx / abs(m_sslCtx);
	return m_sslCtx;
}

int getColor(int piece) {
	return piece & 0b11;
}

int getPieceName(int piece) {
	return piece & 0b1111100;
}

int otherColor(int c) {
	return c ^ 0b11;
}

typedef struct {
	int data[64];
} board;

int getSpot(board* b, int x, int y) {
	return b->data[x + y * 8];
}

void setSpot(board* b, int x, int y, int piece) {
	b->data[x + y * 8] = piece;
}

bool sameColor(board* b, int x, int y, int color) {
	return getColor(getSpot(b, x, y)) == color && getSpot(b, x, y);//same color and there's somethign there
}

board moveSpot(board b, int x1, int y1, int x2, int y2) {
	//en passant
	if (getPieceName(getSpot(&b, x1, y1)) == PAWN && x1 != x2 && !getSpot(&b, x2, y2)) 
		setSpot(&b, x2, y2 - BLACK ? -1 : 1, NONE);

	setSpot(&b, x2, y2, getSpot(&b, x1, y1));
	setSpot(&b, x1, y1, NONE);
	//castleing
	if (getPieceName(getSpot(&b, x2, y2)) == KING && abs(x1 - x2) == 2) {
		int y = getColor(getSpot(&b, x2, y2)) == BLACK ? 0 : 7;
		if (y2 != y || y1 != y)
			return b;
		if (x2 == 6) 
			b = moveSpot(b, 7, y, 5, y);
		else if (x2 == 2) 
			b = moveSpot(b, 0, y, 3, y);		
	}return b;
}

void printPieceSymbol(int piece) {
	char pieceSymbol[7] = { 'P','B','R','K','N','Q' };
	char pieceColors[3] = "wb";
	if (!piece) {
		printf("|    ");
		return;
	}printf("| %c%c ", pieceColors[getColor(piece) / 2], pieceSymbol[(int)log2(double(getPieceName(piece))) - 2]);
}

void printBoard(board* b) {
	printf("    +----+----+----+----+----+----+----+----+\n");
	int x = 0;
	int y = 0;
	for (; y<8; ++y) {
		printf("%i   ", 8 - y);
		for (x = 0; x<8; ++x) 
			printPieceSymbol(getSpot(b, x, y));			
		printf("|\n    +----+----+----+----+----+----+----+----+\n");
	}printf("       a    b    c    d    e    f    g    h");
}

void getKingSpot(board* b, int color, int* kx, int* ky) {
	int x = 0;
	int y = 0;
	for (; x<8; ++x) {		
		for (y = 0; y<8; ++y) {
			if (getSpot(b, x, y) == KING + color) {
				*kx = x;
				*ky = y;
				return;
			}
		}
	}
}

bool isValidSpot(int x, int y) {
	return x >= 0 && x<8 && y >= 0 && y<8;
}

static int bishMoveSpotsx[4] = { -1,-1,1,1 };
static int bishMoveSpotsy[4] = { 1,-1,-1,1 };

static int rookMoveSpotsx[4] = { 1,-1,0,0 };
static int rookMoveSpotsy[4] = { 0,0,1,-1 };

static int knightMoveSpotsx[8] = { 1, 2, 1, 2,-1,-2,-1,-2 };
static int knightMoveSpotsy[8] = { 2, 1,-2,-1, 2, 1,-2,-1 };

static int kingMoveSpotsx[8] = { 1,1,1,0,0,-1,-1,-1 };
static int kingMoveSpotsy[8] = { 1,1,1,0,0,-1,-1,-1 };
//queen is just bish + rook;

bool ray_trace(board* b,int kingX,int kingY,int color,int pieceType,int dx,int dy){//returns weather or not something can hit the king
	int u = 1;
	for (u = 1; isValidSpot(kingX + u * dx, kingY + u * dy) ;++u) {
		if (getSpot(b, kingX + u * dx, kingY + u * dy) == pieceType + color || getSpot(b, kingX + u * dx, kingY + u * dy) == QUEEN + color)
			return true;
		if (getSpot(b, kingX + u * dx, kingY + u * dy))//theres a piece that can't hit the king
			return false;
	}return false;
}

bool isValidBoard(board* b, int lastPlayedColor) {
	int kingXSpot;
	int kingYSpot;
	getKingSpot(b, lastPlayedColor, &kingXSpot, &kingYSpot);
	int color = otherColor(lastPlayedColor);
	int i = 0;
	int u = 1;
	//bish and rook
	for (i; i<4; ++i) {
		bool invalid = ray_trace(b, kingXSpot, kingYSpot, color, BISH, bishMoveSpotsx[i], bishMoveSpotsy[i]) || ray_trace(b, kingXSpot, kingYSpot, color, ROOK, rookMoveSpotsx[i], rookMoveSpotsy[i]);
		if (invalid) return false;		
	}
	//knight and king
	for (i = 0; i<8; ++i) {
		if ((isValidSpot(kingXSpot + kingMoveSpotsx[i],
			kingYSpot + kingMoveSpotsy[i]) &&
			getSpot(b, kingXSpot + kingMoveSpotsx[i], kingYSpot + kingMoveSpotsy[i]) == KING + color) ||
			(isValidSpot(kingXSpot + knightMoveSpotsx[i],
				kingYSpot + knightMoveSpotsy[i]) &&
				getSpot(b, kingXSpot + knightMoveSpotsx[i], kingYSpot + knightMoveSpotsy[i]) == KNIGHT + color))
			return false;
	}
	//pawns
	int dy = color == BLACK ? 1 : -1;
	if (isValidSpot(kingXSpot + 1, kingYSpot + dy) && getSpot(b, kingXSpot + 1, kingYSpot + dy) == PAWN + color) 
		return false;	
	if (isValidSpot(kingXSpot - 1, kingYSpot + dy) && getSpot(b, kingXSpot - 1, kingYSpot + dy) == PAWN + color) 
		return false;	
	return true;
}

bool hmmm(board* b, int dx, int dy, int x1, int y1, int dtotal) {
	int u;
	for (u = 1; u<dtotal; ++u) {
		if (getSpot(b, dx*u + x1, dy*u + y1))
			return false;
	}return true;
}

bool validMoveForRook(board* b, int color, int x1, int y1, int x2, int y2) {
	if (!((x1 == x2) ^ (y1 == y2)) || sameColor(b, x2, y2, color))
		return false;
	return hmmm(b, dv(x2 - x1), dv(y2 - y1), x1, y1, abs(x1 - x2) + abs(y1 - y2));
}

bool validMoveForBish(board* b, int color, int x1, int y1, int x2, int y2) {
	if (abs(x1 - x2) != abs(y1 - y2) || sameColor(b, x2, y2, color))
		return false;
	return hmmm(b, dv(x2 - x1), dv(y2 - y1), x1, y1, abs(x1 - x2));
}

bool validMoveForQueen(board* b, int color, int x1, int y1, int x2, int y2) {
	return validMoveForBish(b, color, x1, y1, x2, y2) || validMoveForRook(b, color, x1, y1, x2, y2);
}

bool validMoveForKing(board* b, int color, int x1, int y1, int x2, int y2) {
	return abs(x1 - x2) <= 1 && abs(y1 - y2) <= 1 && !sameColor(b, x2, y2, color) && abs(x1 - x2) + abs(y1 - y2);
}
bool validMoveForKnight(board* b, int color, int x1, int y1, int x2, int y2) {
	int dx = abs(x1 - x2);
	int dy = abs(y1 - y2);
	return (dx <= 2 && dy <= 2 && dx + dy == 3) && !sameColor(b, x2, y2, color);
}

bool validMoveForPawn(board* b, int color, int x1, int y1, int x2, int y2) {
	if (color == WHITE && y2 >= y1) 
		return false;
	else if (color == BLACK && y2 <= y1) 
		return false;
	
	int dx = abs(x1 - x2);
	int dy = abs(y1 - y2);
	if (dx == 1 && dy == 1) 
		return !sameColor(b, x2, y2,color) && getSpot(b, x2, y2);	
	if (getSpot(b, x2, y2) || dx) 
		return false;	
	if ((color == BLACK && y1 == 1) || (color == WHITE && y1 == 6)) 
		return dy <= 2 && !getSpot(b, x2, y2 + 1);	
	return dy == 1;
}


char kingCastleing = 4;
char rightRookCastleing = 2;
char leftRookCastleing = 1;

bool validKingCastle(board* b, int color, int x1, int y1, int x2, int y2, char castleingStatus) {
	if (castleingStatus & kingCastleing)
		return false;

	int side = x2 == 2 ? leftRookCastleing : rightRookCastleing;
	if ((castleingStatus & side)) 
		return false;	

	int y = color == BLACK ? 0 : 7;
	if (y1 != y2 || y1 != y || (x2 != 2 && x2 != 6) || x1 != 4)
		return false;

	if (side == leftRookCastleing) {
		for(int i = 0;i<3;++i){
			board nb = moveSpot(*b,x1,y,x1 - 1 - i,y2);
			if (!isValidBoard(&nb,color))
				return false;			
		}
		return !getSpot(b, x1 - 1, y) && !getSpot(b, x1 - 2, y) && !getSpot(b, x1 - 3, y);			
	}
	for (int i = 0; i<2; ++i) {
		board nb = moveSpot(*b, x1, y, x1 +  1 + i, y2);
		if (!isValidBoard(&nb, color))
			return false;
	}	
	return !getSpot(b, x1 + 1, y) && !getSpot(b, x1 + 2, y);
}

bool validEnpassant(board* b, int color, int x1, int y1, int x2, int y2, int enpassantStatus) {
	if (enpassantStatus == -1)
		return false;
	int y = color == BLACK ? 5 : 4;
	int dy = color == BLACK ? 1 : -1;
	if (y1 != y || dy + y != y2) 
		return false;	
	return x2 == enpassantStatus;
}

int main() {
	board b;
	for(int i =0;i<64;++i){
		b.data[i] = 0;
	}

	setSpot(&b, 1, 1, PAWN + BLACK);
	setSpot(&b, 4, 0, KING + BLACK);
	setSpot(&b, 4, 7, KING + WHITE);
	setSpot(&b, 1, 6, PAWN + WHITE);
	setSpot(&b, 0, 7, ROOK + WHITE);
	setSpot(&b, 1, 7, KNIGHT + WHITE);
	setSpot(&b, 2, 7, BISH + WHITE);
	setSpot(&b, 7, 7, ROOK + WHITE);
	char check = false;
	char checkmate = false;
	int turns = 0;
	int colourTurn = WHITE;

	char castleing_white_status = 0;
	char castleing_black_status = 0;

	char whiteEnpassantable = -1;
	char blackEnpassantable = -1;

	while (!checkmate) {
		printBoard(&b);
		printf("\n");
		if (check) printf("check\n");
		int x1, y1, x2, y2;
		scanf_s("%i %i %i %i", &x1, &y1, &x2, &y2);
		if (!isValidSpot(x1, y1) || !isValidSpot(x2, y2) || !sameColor(&b, x1, y1, colourTurn)) {
			printf("invalid piece\n");
			continue;
		}
		int pieceName = getPieceName(getSpot(&b, x1, y1));
		bool validMove = false;
		//check for en passant and castleing
		switch (pieceName) {
		case PAWN:
			validMove = validMoveForPawn(&b, colourTurn, x1, y1, x2, y2) ||
				validEnpassant(&b, colourTurn, x1, y1, x2, y2, colourTurn == BLACK ? blackEnpassantable : whiteEnpassantable);
			break;
		case BISH:
			validMove = validMoveForBish(&b, colourTurn, x1, y1, x2, y2);
			break;
		case QUEEN:
			validMove = validMoveForQueen(&b, colourTurn, x1, y1, x2, y2);
			break;
		case KING:
			validMove = validMoveForKing(&b, colourTurn, x1, y1, x2, y2) ||
				validKingCastle(&b, colourTurn, x1, y1, x2, y2, colourTurn == BLACK ? castleing_black_status : castleing_white_status);
			break;
		case KNIGHT:
			validMove = validMoveForKnight(&b, colourTurn, x1, y1, x2, y2);
			break;
		case ROOK:
			validMove = validMoveForRook(&b, colourTurn, x1, y1, x2, y2);
			break;
		default:
			break;
		}
		board newBoard = moveSpot(b, x1, y1, x2, y2);
		validMove = validMove && isValidBoard(&newBoard, colourTurn);
		if (!validMove) {
			printf("invalid move\n");
			continue;
		}
		//pawn promotion
		int endY = colourTurn == BLACK ? 7 : 0;
		if (getSpot(&newBoard, x2, y2) == PAWN + colourTurn && y2 == endY) {
			printf(" ");
			int toBePiece = 0;
			scanf_s("%i", &toBePiece);
			if (toBePiece == 0) {
				setSpot(&b, x2, y2, ROOK + colourTurn);
			}else if (toBePiece == 1) {
				setSpot(&b, x2, y2, BISH + colourTurn);
			}else if (toBePiece == 2) {
				setSpot(&b, x2, y2, KNIGHT + colourTurn);
			}else if (toBePiece == 3) {
				setSpot(&b, x2, y2, QUEEN + colourTurn);
			}else {
				//;-;
			}
		}
		if (x1 == 0 || x2 == 0) {
			if (y1 == 0 || y2 == 0) {
				castleing_black_status |= leftRookCastleing;
			}else if (y1 == 7 || y2 == 7) {
				castleing_black_status |= rightRookCastleing;
			}
		}
		if (x1 == 7 || x2 == 7) {
			if (y1 == 0 || y2 == 0) {
				castleing_white_status |= leftRookCastleing;
			}else if (y1 == 7 || y2 == 7) {
				castleing_white_status |= rightRookCastleing;
			}
		}

		if (y1 == 0 && x1 == 4)
			castleing_black_status |= kingCastleing;
		else if (y1 == 7 && x1 == 4)
			castleing_white_status |= kingCastleing;

		if (getPieceName(getSpot(&b, x1, y1)) == PAWN && abs(y2 - y1) == 2) {
			if (colourTurn == BLACK) {
				blackEnpassantable = x1;
				whiteEnpassantable = -1;
			}
			else {
				whiteEnpassantable = x1;
				blackEnpassantable = -1;
			}
		}
		else {
			blackEnpassantable = -1;
			whiteEnpassantable = -1;
		}

		b = newBoard;
		check = !isValidBoard(&b, otherColor(colourTurn));
		++turns;
		colourTurn = otherColor(colourTurn);		
	}
}


*/



