#pragma once
#include <boost/asio.hpp>

struct resume_on_strand {
	resume_on_strand() = delete;

	explicit resume_on_strand(boost::asio::io_context::strand& s) :
		strand(s) {};

	explicit resume_on_strand(boost::asio::io_context::strand* s) :
		strand(*s) {};
	
	boost::asio::io_context::strand& strand;

	bool await_ready() {
		return false;
	}

	void await_suspend(std::experimental::coroutine_handle<> h) {
		boost::asio::post(strand, h);
	}

	void await_resume() {
		//do nothing
	}

};
