/*
 * Created by Maou Lim on 2017/12/23.
 */

#ifndef _CONCURRENCY_BLOCKING_QUEUE_H_
#define _CONCURRENCY_BLOCKING_QUEUE_H_

#include <deque>
#include <mutex>
#include <condition_variable>

namespace concurrency {

	template <typename _Val>
	class blocking_queue {
	public:
		blocking_queue() = default;

		blocking_queue(const blocking_queue& other) = delete;
		blocking_queue& operator=(const blocking_queue&) = delete;

		void push(const _Val& val) {
			std::lock_guard<std::mutex> locker(m_mtx);
			m_container.push_back(val);
			m_cond.notify_one();
		}

		bool try_pop(_Val& val) {
			std::lock_guard<std::mutex> locker(m_mtx);
			if (m_container.empty()) {
				return false;
			}

			val = m_container.front();
			m_container.pop_front();
			return true;
		}

		std::shared_ptr<_Val> try_pop() {
			std::lock_guard<std::mutex> locker(m_mtx);
			if (m_container.empty()) {
				return std::shared_ptr<_Val>();
			}

			auto ptr = std::make_shared<_Val>(m_container.front());
			m_container.pop_front();
			return ptr;
		}

		void wait_and_pop(_Val& val) {
			std::unique_lock<std::mutex> locker(m_mtx);
			m_cond.wait(locker, [this] { return !m_container.empty(); });
			val = m_container.front();
			m_container.pop_front();
		}

		std::shared_ptr<_Val> wait_and_pop() {
			std::unique_lock<std::mutex> locker(m_mtx);
			m_cond.wait(locker, [this] { return !m_container.empty(); });
			auto ptr = std::make_shared<_Val>(m_container.front());
			m_container.pop_front();
			return ptr;
		}

	private:
		std::mutex              m_mtx;
		std::condition_variable m_cond;
		std::deque<_Val>        m_container;
	};
}

#endif //CONCURRENCY_BLOCKING_QUEUE_H
