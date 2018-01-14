/*
 * Created by Maou Lim on 17-4-3.
 */

#include <future>
#include "concurrency.h"

namespace concurrency {
	namespace fs = std::experimental::filesystem;
	using fut_vec = std::vector<std::future<str_vec>>;

	static const fs::directory_iterator end;

	str_vec single_thread_list(std::string&& root) noexcept {

		concurrency::str_vec files;
		concurrency::str_vec dirs;

		dirs.emplace_back(root);

		while (!dirs.empty()) {
			auto dir = std::move(dirs.back());
			dirs.pop_back();

			for (fs::directory_iterator itr(dir); itr != end; ++itr) {
				if (fs::is_directory(*itr)) {
					dirs.emplace_back(itr->path().generic_string());
					continue;
				}

				files.emplace_back(itr->path().filename().generic_string());
			}
		}

		return files;
	}

	str_vec unbounded_thread_list(std::string&& root) noexcept {
		concurrency::str_vec files;
		concurrency::fut_vec futures;

		for (fs::directory_iterator itr(root); itr != end; ++itr) {
			if (fs::is_directory(*itr)) {
				auto fut = std::async(std::launch::async,
				                      unbounded_thread_list,
				                      itr->path().generic_string());
				futures.emplace_back(std::move(fut));
				continue;
			}

			files.emplace_back(itr->path().filename().generic_string());
		}

		for (auto& each : futures) {
			auto child_files = std::move(each.get());
			std::copy(child_files.begin(), child_files.end(), std::back_inserter(files));
		}

		return files;
	}

	static constexpr unsigned int MAX_TASKS_NUM = 8;

	struct dir_or_file {
		dir_or_file() = default;

		dir_or_file(dir_or_file&& other) noexcept :
			files(std::move(other.files)),
			dirs(std::move(other.dirs)) { }

		dir_or_file& operator=(dir_or_file&& other) noexcept {
			files = std::move(other.files);
			dirs = std::move(other.dirs);
		}

		str_vec files;
		str_vec dirs;
	};

	static dir_or_file _list_task(std::string&& root) {
		dir_or_file res;
		for (fs::directory_iterator itr(root); itr != end; ++itr) {
			if (fs::is_directory(*itr)) {
				res.dirs.emplace_back(itr->path().generic_string());
				continue;
			}

			res.files.emplace_back(itr->path().filename().generic_string());
		}

		return res;
	}

	str_vec bounded_thread_list(std::string&& root) noexcept {
		std::vector<std::future<dir_or_file>> futures;
		dir_or_file result;

		result.dirs.emplace_back(root);

		while (!result.dirs.empty()) {

			for (unsigned int i = 0u; i < MAX_TASKS_NUM && !result.dirs.empty(); ++i) {
				auto dir = std::move(result.dirs.back());
				result.dirs.pop_back();

				auto fut = std::async(std::launch::async, _list_task, std::move(dir));
				futures.emplace_back(std::move(fut));
			}

			try {
				while (!futures.empty()) {
					auto fut = std::move(futures.back());
					futures.pop_back();

					auto res = std::move(fut.get());
					std::move(res.files.begin(), res.files.end(), std::back_inserter(result.files));
					std::move(res.dirs.begin(), res.dirs.end(), std::back_inserter(result.dirs));
				}
			}

			catch (...) {
				exit(-1);
			}
		}

		return result.files;
	}

	class thread_safe_df {
	public:
		thread_safe_df() = default;

		void push_file(std::string&& file) {
			std::lock_guard<std::mutex> lock(m_mutex);
			m_content.files.emplace_back(file);
			m_cond.notify_all();
		}

		void peek_file(std::string& file) {
			std::unique_lock<std::mutex> lock(m_mutex);
			m_cond.wait(lock, [&]() { return !m_content.files.empty(); });
			file = std::move(m_content.files.back());
			m_content.files.pop_back();
		}

		void push_dir(std::string&& dir) {
			std::lock_guard<std::mutex> lock(m_mutex);
			m_content.dirs.emplace_back(dir);
			m_cond.notify_all();
		}

		void peek_dir(std::string& dir) {
			std::unique_lock<std::mutex> lock(m_mutex);
			m_cond.wait(lock, [&]() { return !m_content.dirs.empty(); });
			dir = std::move(m_content.dirs.back());
			m_content.dirs.pop_back();
		}

	private:
		dir_or_file m_content;
		std::condition_variable m_cond;
		std::mutex m_mutex;
	};

	static void _list_task2(std::string&& root, thread_safe_df& df) {
		for (fs::directory_iterator itr(root); itr != end; ++itr) {
			if (fs::is_directory(*itr)) {
				df.push_dir(std::move(itr->path().generic_string()));
				continue;
			}

			df.push_file(itr->path().filename().generic_string());
		}
	}

	str_vec shared_memory_list(std::string&& root) noexcept {
		thread_safe_df df;



		return str_vec();
	}
}















