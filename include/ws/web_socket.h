#pragma once
#include "stuff.h"
#include "networking_coro_stuff.h"
#include <string_view>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include "randomThings.h"
#include "task_completion_handler.h"
#include <memory>

inline std::vector<unsigned char> prepare_msg(const std::string& msg,opcode o,bool is_client) {
	std::vector<unsigned char> ret;
	ret.reserve(msg.size() + 20);
	auto size = msg.size();
	ret.push_back((u_char)0b10000000 + (u_char)o);//fin + opcode
	if(msg.size() <=125) {
		ret.push_back((u_char)((is_client << 7) + msg.size()));
	}else if(msg.size()>65535) {
		ret.push_back((u_char)((is_client << 7) + 127));
	}else {//msg is in [126,65535]
		ret.push_back((u_char)((is_client << 7) + 126));
		//;-;
	}
	const auto old_size = ret.size();
	ret.resize(ret.size() + msg.size());
	std::copy(std::execution::par_unseq, msg.begin(), msg.end(), ret.begin() + old_size);

	return ret;
}

template<typename ...args>
struct send_handler:base_but_also_stores_stuff<args...>{
	send_handler(args... stuff):base_but_also_stores_stuff<args...>(std::move(stuff)...){}
};

template<typename...args>
send_handler(args...)->send_handler<std::decay_t<args>...>;

template<typename handler,typename sig>
class boost::asio::async_result<send_handler<handler, std::string>, sig> :public boost::asio::async_result<handler, sig>{
public:
	//using boost::asio::async_result<handler, sig>::boost::asio::async_result<handler, sig>;
	template<typename T>
	async_result(T&& a):boost::asio::async_result<handler, sig>(std::forward<T>(a)){}
};

template<typename socket_type,typename hub_type,typename... other_stuff>
struct web_socket:std::enable_shared_from_this<web_socket<socket_type,hub_type>>,other_stuff... {
	explicit web_socket(socket_type sock, hub_type* parent) :
		m_socket(std::move(sock)), m_parent(parent) {
		
	};

	web_socket(web_socket&&) = default;
	web_socket(const web_socket&) = delete;

	web_socket& operator=(web_socket&&) = default;
	web_socket& operator=(const web_socket&) = delete;

	~web_socket() {
		try{
		if(is_open())
			close(4000);
		}catch(...){}
	}
	
	std::future<boost::system::error_code> async_send(std::string msg) {
		to_not_sso_string(msg);//sso string messes things up after being moved, since msg.data will be pointing to the stack
		const auto[ptr, size] = std::make_pair(msg.data(), msg.size());
		std::promise<boost::system::error_code> p;
		auto ret = p.get_future();
		m_socket.async_write(boost::asio::buffer(ptr, size), [pin_ = std::move(msg), p_ = std::move(p)](auto ec, size_t n)mutable {
			p_.set_value(std::move(ec));
		});
		return ret;
	}

	template<typename send_handler_t>
	decltype(auto) async_send(std::string msg, send_handler_t&& handler) {
		m_socket.text(true);
		to_not_sso_string(msg);
		return m_socket.async_write(boost::asio::buffer(msg.data(),msg.size()), send_handler(std::forward<send_handler_t>(handler), std::move(msg)));
	}

	template<typename send_handler_t>
	decltype(auto) async_send_binary(std::string msg, send_handler_t&& handler) {
		m_socket.binary(true);
		to_not_sso_string(msg);
		return m_socket.async_write(boost::asio::buffer(msg.data(),msg.size()), send_handler(std::forward<send_handler_t>(handler), std::move(msg)));
	}
	
	void send(const std::string_view msg) {
		m_socket.write(boost::asio::buffer(msg));
	}

	void close(const int code) {		
		m_socket.close(boost::beast::websocket::close_reason(code));
		m_parent->on_close(*this);
	}

	auto& socket() noexcept{
		return m_socket;
	}

	const auto& socket() const noexcept{
		return m_socket;
	}

	void start_async_read() {
		auto me = this->shared_from_this();
		m_socket.async_read(m_buffer,[this,me = this->shared_from_this()](const auto& ec,size_t n){
			if(!ec){
				std::string msg;
				msg.reserve(n + 4);//idk ;-;
				msg.resize(n + 1);
				auto it = msg.begin();
				for(const auto& i:m_buffer.data()) 
					it = std::copy((char*)i.data(), (char*)i.data() + i.size(), it);
				
				if (m_socket.got_text())
					m_parent->on_msg(*this, std::string_view(msg.data(), n));
				else
					m_parent->on_binary(*this, std::string_view(msg.data(), n));
				m_buffer.consume(n);
				start_async_read();
			}else {
				std::cout << ec << std::endl;
			}
		});
	}

	task<void> start_reads() {
		auto me = this->shared_from_this();
		try{
			while(true) {
				size_t n = co_await m_socket.async_read(m_buffer, use_task);
				std::string msg;
				msg.reserve(n + 4);//idk ;-;
				msg.resize(n + 1);
				auto it = msg.begin();
				for (const auto& i : m_buffer.data()) {
					it = std::copy((char*)i.data(), (char*)i.data() + i.size(), it);
				}
				if (m_socket.got_text())
					m_parent->on_msg(*this, msg);
				else
					m_parent->on_binary(*this, msg);
				m_buffer.consume(n);
			}
		}catch(...){}
	}
	
	void enable_deflate() {
		boost::beast::websocket::permessage_deflate pmd;
		pmd.client_enable = true;
		pmd.server_enable = true;
		pmd.compLevel = 3;
		m_socket.set_option(pmd);
	}

	void disable_deflate() {
		boost::beast::websocket::permessage_deflate pmd;
		pmd.client_enable = false;
		pmd.server_enable = false;
		m_socket.set_option(pmd);
	}
	bool is_open() {
		return m_socket.is_open();
	}
private:
	boost::beast::websocket::stream<socket_type> m_socket;
	boost::beast::multi_buffer m_buffer = {};
	hub_type* m_parent = nullptr;
};

template<typename socket_type,typename hub_type>
task<void> do_session(web_socket<socket_type,hub_type> sock,hub_type* m_parent) {

	boost::beast::multi_buffer m_buffer = {};
	auto& m_socket = sock.socket();
	try{
	while(true) {
		size_t n = co_await m_socket.async_read(m_buffer, use_task);
		std::string msg;
		msg.reserve(n + 4);//idk ;-;
		msg.resize(n + 1);
		auto it = msg.begin();
		for (const auto& i : m_buffer.data()) {
			it = std::copy((char*)i.data(), (char*)i.data() + i.size(), it);
		}
		if (m_socket.got_text())
			m_parent->on_msg(sock, msg);
		else
			m_parent->on_binary(sock, msg);
		m_buffer.consume(n);
	}

	}catch(...) {
		//socket dies
		m_parent->on_close(sock);
	}
}

/*
 *std::string_view data(m_buffer.data(), n);
			
			opcode op = (opcode)(data[0] & 0b1111);
			if (op != opcode::continuation)
				m_current_opcode = op;

			const bool is_fin = data[0] & 0b10000000;
			data.remove_prefix(1);

			bool has_masking_frame = data[0] & 0b10000000;
			int payload_len = (int)(data[0] & 0b01111111);
			data.remove_prefix(1);

			if (payload_len == 126)
				payload_len = (int)read_n_bytes(2, data);
			else if (payload_len == 127)
				payload_len = (int)read_n_bytes(8, data);

			unsigned masking_key = 0;
			if (has_masking_frame)
				masking_key = (unsigned)read_n_bytes(4, data);

			int extension_data_len = 0;
			// TO DO extensions, maybe
			int application_data_len = payload_len - extension_data_len;

			int old_end = (int)m_current_msg.size();
			m_current_msg.resize(m_current_msg.size() + application_data_len);
			
			const std::array<unsigned, 4> masking_frame = {(masking_key >>24) & 0xff,(masking_key>>16) & 0xff ,(masking_key >> 8) & 0xff ,(masking_key >> 0) & 0xff };

			//auto start = (int*)data.data();
			//auto end = (int*)(data.data() + data.size());
			
			for(int i = 0;i<(int)data.size();++i) {//transform ;-;
				m_current_msg[i + old_end] = data[i] ^ (unsigned char)masking_frame[(i % 4)];
			}

			//std::vector<unsigned> masked_data(data.size() * 4);
			//std::transform(std::execution::par_unseq,start, end, masked_data.begin(), [&](auto i) {return i ^ masking_key; });

			if(is_fin){
				m_parent->on_msg(*this, m_current_msg, m_current_opcode);
				m_current_msg.clear();
			}
 *
 */