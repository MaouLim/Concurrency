/*
 * Created by Maou Lim on 2017/11/21.
 */

#ifndef _CONCURRENCY_THREAD_GUARD_H_
#define _CONCURRENCY_THREAD_GUARD_H_

#include <thread>

namespace concurrency {
	/*
	 * thread_guard 用于保证线程的正确退出，防止线程继续访问局部资源
	 */
	class thread_guard {
	public:
		explicit thread_guard(std::thread&& thd) :
			m_thd(std::move(thd)) { }

		thread_guard(const thread_guard&) = delete;

		thread_guard(thread_guard&& other) :
			m_thd(std::move(other.m_thd)) { }

		~thread_guard() {
			if (m_thd.joinable()) {
				m_thd.join();
			}
		}

		thread_guard& operator=(const thread_guard&) = delete;

		thread_guard& operator=(thread_guard&& other) {
			if (this == &other) {
				return *this;
			}

			m_thd = std::move(other.m_thd);
			return *this;
		}

	private:
		std::thread m_thd;
	};
}

#endif //CONCURRENCY_THREAD_GUARD_H
