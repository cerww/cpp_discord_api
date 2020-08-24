#pragma once
#include <boost/asio.hpp>
#include <range/v3/all.hpp>


//switch to std::find and swap->pop_back for removing, push_back for adding?

template<typename bucket_t, typename obj_t, typename clock_t = std::chrono::steady_clock>
struct rate_limiter {
	explicit rate_limiter(boost::asio::io_context& ioc):
		m_ioc(ioc) {}

	//returns whether or not it was rate limited, moves thing if it was
	bool maybe_rate_limit(const bucket_t& bucket, obj_t& thing) {
		std::lock_guard lock(m_mut);
		auto it = ranges::find(m_entries, bucket, &rate_limit_entry::bucket);
		if (it == m_entries.end()) {
			return false;
		} else {
			(*it).objs_rate_limited.push_back(std::move(thing));
			return true;
		}
	}
	
	//pre: not bucket isn't already rate-limited
	void rate_limit(const bucket_t& bucket, typename clock_t::time_point until) {
		std::lock_guard lock(m_mut);
		auto where = ranges::upper_bound(m_entries, bucket, std::less{}, &rate_limit_entry::bucket);
		m_entries.insert(where, rate_limit_entry{bucket, until, {}});
		make_timer_for_bucket(bucket, until);
	}

	void rate_limit(const bucket_t& bucket, typename clock_t::time_point until, obj_t obj) {
		//make vector before locking
		std::vector<obj_t> v;
		v.emplace_back(std::move(obj));// do this instead of initializer_list because initializer_list will copy

		std::lock_guard lock(m_mut);
		auto where = ranges::upper_bound(m_entries, bucket, std::less{}, &rate_limit_entry::bucket);
		m_entries.insert(where, rate_limit_entry{bucket, until, v});
		make_timer_for_bucket(bucket, until);

	}

	std::function<void(std::vector<obj_t>)> on_rate_limit_finish = [](auto&&...) {};

private:

	void make_timer_for_bucket(const bucket_t& bucket, typename clock_t::time_point until) {
		auto timer = std::make_unique<boost::asio::basic_waitable_timer<clock_t>>(m_ioc);
		timer->expires_at(until);
		timer->async_wait([timer_ = std::move(timer), this, bucket_ = bucket](auto ec) {
			if (!ec) {
				auto objs_to_un_rate_limit = [&]() {
					std::lock_guard lock(m_mut);
					auto it = ranges::lower_bound(m_entries, bucket_, std::less(), &rate_limit_entry::bucket);					
					auto ret = std::move(it->objs_rate_limited);
					m_entries.erase(it);
					return ret;
				}();
				on_rate_limit_finish(std::move(objs_to_un_rate_limit));
			}
		});
	}

	struct rate_limit_entry {
		bucket_t bucket;
		typename clock_t::time_point rate_limted_until;
		std::vector<obj_t> objs_rate_limited;
	};

	boost::asio::io_context& m_ioc;
	std::mutex m_mut;
	//sorted by bucket
	std::vector<rate_limit_entry> m_entries;
};
