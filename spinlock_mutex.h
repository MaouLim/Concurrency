/*
 * Created by Maou Lim on 2018/1/7.
 */

#ifndef _CONCURRENCY_SPINLOCK_MUTEX_H_
#define _CONCURRENCY_SPINLOCK_MUTEX_H_

#include <atomic>

namespace concurrency {

	class spinlock_mutex {
	public:
		spinlock_mutex() : m_flag(ATOMIC_FLAG_INIT) { }

		void lock() {
			while (m_flag.test_and_set(std::memory_order_acquire));
		}

		void unlock() {
			m_flag.clear(std::memory_order_release);
		}

	private:
		std::atomic_flag m_flag;
	};
}

#endif //CONCURRENCY_SPINLOCK_MUTEX_H
