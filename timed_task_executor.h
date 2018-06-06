#pragma once
#include <thread>
#include <atomic>
#include <functional>
#include <algorithm>

struct timed_task_executor{
	timed_task_executor() {
		m_thread = std::thread([this](){
			while(!m_done) {
				std::unique_lock<std::mutex> locky(m_mut);
				if (!m_tasks.empty()) {
					if (m_cv.wait_until(locky,m_tasks[0].second, [&]() {return m_tasks[0].second >= std::chrono::steady_clock::now(); })) 
						execute_task();					
				}else {
					m_cv.wait(locky);
					if(m_tasks[0].second >=std::chrono::steady_clock::now()) 
						execute_task();					
				}
					
			}
		});
	}

	timed_task_executor& operator=(const timed_task_executor&) = delete;
	timed_task_executor& operator=(timed_task_executor&&) = delete;
	timed_task_executor(const timed_task_executor&) = delete;
	timed_task_executor(timed_task_executor&&) = delete;

	~timed_task_executor() {
		m_done.store(true);
		m_thread.join();
	}
	void execute(std::pair<std::function<void()>,std::chrono::steady_clock::time_point> task) {
		{
		std::lock_guard<std::mutex> locky(m_mut);
		auto it = std::upper_bound(m_tasks.begin(),m_tasks.end(),task,[](const auto& a,const auto& b){
			return a.second.count() < b.second.count();
		});
		m_tasks.insert(it,std::move(task));
		}
		m_cv.notify_one();
	}

private:
	void execute_task() {
		auto t = std::move(m_tasks[0]);
		m_tasks.erase(m_tasks.begin());
		t.first();
	}
	std::thread m_thread;
	std::atomic<bool> m_done = false;
	std::mutex m_mut;
	std::condition_variable m_cv;
	std::vector<std::pair<std::function<void()>, std::chrono::steady_clock::time_point>> m_tasks = {};
};
