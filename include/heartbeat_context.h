#pragma once
#include "rename_later_5.h"

struct internal_shard;

struct heartbeat_context {
	
	explicit heartbeat_context(internal_shard& shard):m_shard(shard){}
	
	void start();

	int hb_interval = 0;

	std::atomic<bool> recived_ack = false;
	
private:
	
	void send_heartbeat();

	internal_shard& m_shard;
	rename_later_5* m_socket = nullptr;	
};