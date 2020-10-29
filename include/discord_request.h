#pragma once
#include <boost/beast.hpp>
#include <optional>

struct discord_request {
	boost::beast::http::request<boost::beast::http::string_body> req;	
	std::optional<std::function<void(boost::beast::http::response<boost::beast::http::string_body>)>> on_finish = std::nullopt;
	//include major_param_id here too
};

constexpr static int aaushdkjasdjaskldasd = sizeof(discord_request);



