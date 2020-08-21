#pragma once
#include "../client.h"


template<typename duration>
cerwy::task<void> prune_every_x_time(shard& s,duration d) {
	while(true) {
		boost::asio::steady_timer timer(s.strand());
		timer.expires_after(d);
		co_await timer.async_wait(use_task);		
		s.clear_deleted_data_if([now = std::chrono::steady_clock::now(), d_ = d](std::chrono::steady_clock::time_point tp,auto&&) {
			return now - tp < d_;
		});
	}
}

template<typename duration,typename pred>
cerwy::task<void> prune_every_x_time(shard& s, duration d,pred&& p) {
	while (true) {
		boost::asio::steady_timer timer(s.strand());
		timer.expires_after(d);
		co_await timer.async_wait(use_task);
		s.clear_deleted_data_if([&](auto t,const auto& b) {
			return p(t, b);
		});
	}
}
