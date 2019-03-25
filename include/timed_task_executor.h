#pragma once
#include <thread>
#include <atomic>
#include <functional>
#include <algorithm>
#include "timer_queue.h"

struct timed_task_executor{
	timed_task_executor(){
		m_thread = std::thread([this](){
			try {
				while (!m_done.load()) {
					auto t = m_queue.try_pop_next_time_blocking();
					if (t.first) {
						std::invoke(*t.first);
					}
					if (t.second) {
						std::this_thread::sleep_until(*t.second);
					}
				}
			}catch(...){}
		});
	}

	timed_task_executor& operator=(const timed_task_executor&) = delete;
	timed_task_executor& operator=(timed_task_executor&&) = delete;
	timed_task_executor(const timed_task_executor&) = delete;
	timed_task_executor(timed_task_executor&&) = delete;

	~timed_task_executor() {
		m_done.store(true);
		execute([]() {}, std::chrono::steady_clock::now() - std::chrono::seconds(1));//to trigger the cv
		m_thread.join();
	}

	void execute(std::pair<std::function<void()>,std::chrono::steady_clock::time_point> task) {
		m_queue.push(std::move(task.first),task.second);
	}

	void execute(std::function<void()> task,std::chrono::steady_clock::time_point tp) {
		execute(std::make_pair(std::move(task), tp));
	}

	template<typename duration>
	void execute_after(std::pair<std::function<void()>, duration> task) {
		execute(std::make_pair(std::move(task.first), task.second + std::chrono::steady_clock::now()));
	}

	template<typename duration>
	void execute_after(std::function<void()> task, duration d) {
		execute(std::make_pair(std::move(task), d + std::chrono::steady_clock::now()));
	}

private:
	std::thread m_thread{};
	std::atomic<bool> m_done = false;
	concurrent_timer_queue<std::function<void()>> m_queue{};
};
