/*
 * Created by Maou Lim on 2017/12/6.
 */

#ifndef _CONCURRENCY_HIERARCHICAL_MUTEX_H_
#define _CONCURRENCY_HIERARCHICAL_MUTEX_H_

#include <mutex>

namespace concurrency {

	class hierarchical_mutex {
	public:
		explicit hierarchical_mutex(unsigned long val) :
			hierarchy_value(val), m_prev_val(0) { }

		void lock() {
			_check_for_hierarchy_violation();
			m_internal.lock();
			_update_hierarchy_value();
		}

		void unlock() {
			this_thread_hierarchy_value = m_prev_val;
			m_internal.unlock();
		}

		bool try_lock() {
			_check_for_hierarchy_violation();
			if (!m_internal.try_lock()) {
				return false;
			}
			_update_hierarchy_value();
			return true;
		}

	private:
		void _check_for_hierarchy_violation() {
			if (this_thread_hierarchy_value <= hierarchy_value) {
				throw std::logic_error("mutex hierarchy violated.");
			}
		}

		void _update_hierarchy_value() {
			m_prev_val = this_thread_hierarchy_value;
			this_thread_hierarchy_value = hierarchy_value;
		}

	private:
		static thread_local unsigned long this_thread_hierarchy_value;

		const unsigned long hierarchy_value;

		std::mutex    m_internal;
		unsigned long m_prev_val;
	};

	thread_local unsigned long hierarchical_mutex::this_thread_hierarchy_value(ULONG_MAX);
}

#endif //CONCURRENCY_HIERARCHICAL_MUTEX_H
