#pragma once
#include <boost/asio.hpp>
#include "task.h"
#include "async_mutex.h"


template<typename T>
struct async_queue2 {
	
	explicit async_queue2(boost::asio::io_context& ioc):
		m_ioc(ioc){}

	
private:
	boost::asio::io_context& m_ioc;
	async_mutex m_mut;
	std::vector<T> m_data;
};