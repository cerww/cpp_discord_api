#pragma once
#include "internal_shard.h"


//gotta move functions outside of internal_shard.cpp, it's too big to compile without /bigobj on vs;-;
cerwy::task<void> init_shard(
	int shardN,
	internal_shard& t_parent,
	boost::asio::io_context& ioc, 
	std::string_view gateway
);



/*
TODO:
put the socket in shard
put create_shard in shard, rename it
put rename_later_5 in shard, rename it
change shard constructor to shard(int,client*,ioc&)

create and add std::make_unique<shard>(n,this,m_ioc)s in to a m_shards var in client

*/