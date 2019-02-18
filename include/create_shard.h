#pragma once
#include "shard.h"


//gotta move functions outside of shard.cpp, it's too big to compile on vs;-;

cerwy::task<void> create_shard(
	int shardN,
	client* t_parent,
	boost::asio::io_context& ioc, 
	std::string_view gateway
);
