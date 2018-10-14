#pragma once
#include <functional>
#include "randomThings.h"
#include <execution>

static const auto nothing = [](auto&&...) {};//so i don't get std::bad_function

enum class hub_type_t{
	client,
	server
};

constexpr size_t read_n_bytes(int n, std::string_view& data) {
	size_t r = 0;
	for (int i = 0; i<n; ++i)
		r += data[i] * std::pow(256, n - i - 1);
	data.remove_prefix(n);
	return r;
}

constexpr size_t read_n_bytes(int n, std::basic_string_view<unsigned char>& data) {
	size_t r = 0;
	for (int i = 0; i<n; ++i)
		r += data[i] * std::pow(256, n - i - 1);
	data.remove_prefix(n);
	return r;
}


enum class protocols {
	chat,
	superchat,
	max
};

inline protocols string_to_protocols(std::string_view s) {
	if (s == "chat") return protocols::chat;
	if (s == "superchat") return protocols::superchat;

	return {};
}

enum class opcode{
	continuation,
	text,
	binary,
	close = 8,
	ping = 9,
	pong = 10
};

template<typename parent,typename web_socket_type>
struct hub_base:private crtp<parent>{

	std::function<void(web_socket_type&, std::string_view)> on_binary = nothing;
	std::function<void(web_socket_type&, std::string_view)> on_msg = nothing;
	std::function<void(web_socket_type&)> on_connect = nothing;
	std::function<void(web_socket_type&)> on_close = nothing;
protected:
	friend typename web_socket_type;
};

template<typename trait,typename = void>
struct is_coroutine_create_socket:std::false_type{};

template<typename trait>
struct is_coroutine_create_socket<trait,std::enable_if_t<trait::is_coroutine_create_socket>> :std::false_type {};

struct has_ioc {
protected:
	auto& p_get_ioc() {
		return m_ioc;
	}
private:
	boost::asio::io_context m_ioc;
};









