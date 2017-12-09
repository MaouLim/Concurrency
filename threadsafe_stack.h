/*
 * Created by Maou Lim on 2017/12/5.
 */

#ifndef _CONCURRENCY_THREADSAFE_STACK_H_
#define _CONCURRENCY_THREADSAFE_STACK_H_

#include <exception>
#include <memory>
#include <stack>
#include <mutex>

namespace concurrency {

	struct empty_stack : std::exception {

		const char* what() const override {
			return "empty stack.";
		}
	};

	template <typename _Val>
	class threadsafe_stack {
	public:
		threadsafe_stack() = default;

		threadsafe_stack(const threadsafe_stack& other) {
			std::lock_guard<std::mutex> locker(other.m_mutex);
			m_container = other.m_container;
		}

		threadsafe_stack& operator=(const threadsafe_stack&) = delete;

		bool empty() const {
			std::lock_guard<std::mutex> locker(m_mutex);
			return m_container.empty();
		}

		void push(const _Val& val) {
			std::lock_guard<std::mutex> locker(m_mutex);
			m_container.push(val);
		}

		std::shared_ptr<_Val> pop() {
			std::lock_guard<std::mutex> locker(m_mutex);
			if (m_container.empty()) {
				throw empty_stack();
			}

			auto ptr = std::make_shared<_Val>(m_container.top());
			m_container.pop();
			return ptr;
		}

		void pop(_Val& result) {
			std::lock_guard<std::mutex> locker(m_mutex);
			if (m_container.empty()) {
				throw empty_stack();
			}

			result = m_container.top();
			m_container.pop();
		}

	private:
		std::stack<_Val>   m_container;
		mutable std::mutex m_mutex;
	};

}

#endif //CONCURRENCY_THREADSAFE_STACK_H
