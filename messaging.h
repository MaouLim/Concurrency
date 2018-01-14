/*
 * Created by Maou Lim on 17-4-5.
 */

#ifndef _CONCURRENCY_MESSAGING_H_
#define _CONCURRENCY_MESSAGING_H_

#include <mutex>
#include <condition_variable>
#include <queue>
#include <cassert>

namespace concurrency::messaging {

	struct message_base {
		virtual ~message_base() = default;
	};

	template <typename ContentType>
	class wrapped_message : public message_base {
	public:
		explicit wrapped_message(const ContentType& content) :
			m_content(content) { }

		explicit wrapped_message(ContentType&& content) :
			m_content(std::move(content)) { }

		const ContentType& content() const {
			return m_content;
		}

	private:
		ContentType m_content;
	};

	template <>
	class wrapped_message<void> {
		wrapped_message() = default;
	};

	struct close_signal { };

	typedef wrapped_message<close_signal> close_message;

	class message_queue {
	public:
		message_queue() = default;

		template <typename ContentType>
		void push(const ContentType& msg_content) {
			std::lock_guard<std::mutex> lock(m_mutex);
			m_queue.push(std::make_shared<wrapped_message<ContentType>>(msg_content));
			m_cond.notify_all();
		}

		std::shared_ptr<message_base> pop() {
			std::unique_lock<std::mutex> lock(m_mutex);
			m_cond.wait(lock, [&]() -> bool { return !m_queue.empty(); });

			auto msg = std::move(m_queue.front());
			m_queue.pop();

			return msg;
		}

	private:
		std::mutex m_mutex;
		std::condition_variable m_cond;
		std::queue<std::shared_ptr<message_base>> m_queue;
	};

	template <typename DispatcherType,
		typename MessageType,
		typename FuncType>
	class temp_dispatcher {
	public:
		template <typename OtherDispatcher,
			typename OtherMessage,
			typename OtherFunc>
		friend class temp_dispatcher;

		temp_dispatcher(temp_dispatcher&& other) :
			m_queue(other.m_queue),
			m_prev(other.m_prev),
			m_func(std::move(other.m_func)),
			m_chained(other.m_chained) {
			other.m_queue = nullptr;
			other.m_prev = nullptr;
			other.m_chained = true;
		}

		temp_dispatcher(message_queue* queue,
		                DispatcherType* prev,
		                FuncType&& func) :
			m_queue(queue),
			m_prev(prev),
			m_func(std::forward<FuncType>(func)), m_chained(false) {
			assert(nullptr != m_queue && nullptr != m_prev);
			m_prev->m_chained = true;
		}

		temp_dispatcher(const temp_dispatcher&) = delete;

		~temp_dispatcher() {
			if (!m_chained) {
				_wait_for_dispatch();
			}
		}

		temp_dispatcher& operator=(const temp_dispatcher&) = delete;

	private:
		bool _dispatch(const std::shared_ptr<message_base>& msg) {
			wrapped_message<MessageType>* wrapped = nullptr;
			if (nullptr != (wrapped = dynamic_cast<wrapped_message<MessageType>*>(msg.get()))) {
				m_func(wrapped->content());
				return true;
			}

			return m_prev->dispatch(msg);
		}

		void _wait_for_dispatch() {
			bool looping = true;

			while (looping) {
				auto msg = m_queue->pop();
				if (this->_dispatch(msg)) {
					looping = false;
				}
			}
		}

	private:
		message_queue* m_queue;
		DispatcherType* m_prev;
		FuncType m_func;
		bool m_chained;
	};

	class dispatcher {
	public:
		template <typename DispatcherType,
			typename MessageType,
			typename FuncType>
		friend class temp_dispatcher;

		explicit dispatcher(message_queue* queue) :
			m_queue(queue),
			m_chained(false) {
			assert(nullptr != m_queue);
		}

		dispatcher(dispatcher&& other) noexcept :
			m_queue(other.m_queue),
			m_chained(other.m_chained) {
			other.m_queue = nullptr;
			other.m_chained = true;
		}

		~dispatcher() noexcept(false) {
			if (!m_chained) {
				_wait_for_dispatch();
			}
		}

		template <typename MessageType, typename FuncType>
		temp_dispatcher<dispatcher, MessageType, FuncType>
		handle(FuncType&& func) {
			return temp_dispatcher<dispatcher, MessageType, FuncType>(
				m_queue, this, std::forward<FuncType>(func)
			);
		};

	private:
		bool _dispatch(const std::shared_ptr<message_base>& msg) throw (close_signal) {
			if (nullptr != dynamic_cast<close_message*>(msg.get())) {
				throw close_signal();
			}

			return false;
		}

		void _wait_for_dispatch() {
			bool looping = true;

			while (looping) {
				auto msg = m_queue->pop();
				_dispatch(msg);
			}
		}

	private:
		message_queue* m_queue;
		bool m_chained;
	};

	class sender {
	public:
		explicit sender(message_queue* queue = nullptr) :
			m_queue(queue) { }

		template <typename ContentType>
		void send(const ContentType& content) {
			if (nullptr == m_queue) {
				return;
			}

			m_queue->push(content);
		}

	private:
		message_queue* m_queue;
	};

	class receiver {
	public:
		receiver() = default;

		operator sender() {
			return sender(&m_queue);
		}

		dispatcher wait() {
			return dispatcher(&m_queue);
		}

	private:
		message_queue m_queue;
	};

}

#endif //_CONCURRENCY_MESSAGING_H_
