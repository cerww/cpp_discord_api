#pragma once
#include "shard.h"


//gotta move functions outside of shard.cpp, it's too big to compile on vs;-;
cerwy::task<void> create_shard(
	int shardN,
	client* t_parent,
	boost::asio::io_context& ioc, 
	std::string_view gateway
);



/*
TODO:
put the socket in shard
put create_shard in shard, rename it
put rename_later_5 in shard, rename it
change shard constructor to shard(int,client*,ioc&)

create and add std::unique_ptr<shard>(n,this,m_ioc)s in to a m_shards var in client

*/