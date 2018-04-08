#pragma once
#include "Channel.h"
#include "User.h"
#include "timestamp.h"
#include <nlohmann/json.hpp>
#include "partial_message.h"
#include <deque>
#include "item_cache.h"

class dm_channel:public Channel{
public:
	snowflake last_message_id() const noexcept;
	std::vector<User>& recipients() noexcept;
	const std::vector<User>& recipients() const noexcept;
	timestamp last_pin_timestamp() const noexcept;

	const std::vector<dm_message>& msg_cache() const noexcept;
	std::vector<dm_message>& msg_cache() noexcept;
private:	
	snowflake m_last_message_id;
	std::vector<User> m_recipients;
	timestamp m_last_pin_timestamp;

	dynamic_item_cache<dm_message> m_msg_cache = dynamic_item_cache<dm_message>(15,false);

	void m_add_msg(dm_message msg);

	friend void from_json(const nlohmann::json& json, dm_channel& channel);
	friend class shard;
};

void to_json(nlohmann::json& json, const dm_channel& channel);


void from_json(const nlohmann::json& json, dm_channel& channel);
