#pragma once
#include "../common/crtp_stuff.h"
#include <nlohmann/json.hpp>

struct modifies_message_json{
	
};

template<typename T>
concept message_modifier = requires(T t,nlohmann::json j){
	{t.modify_message_json(j)};
};

