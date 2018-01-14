/*
 * Created by Maou Lim on 17-4-3.
 */

#ifndef _CONCURRENCY_H_
#define _CONCURRENCY_H_

#include <experimental/filesystem>

namespace concurrency {

	using str_vec = std::vector<std::string>;

	str_vec single_thread_list(std::string&& root) noexcept;

	str_vec unbounded_thread_list(std::string&& root) noexcept;

	str_vec bounded_thread_list(std::string&& root) noexcept;

	str_vec shared_memory_list(std::string&& root) noexcept;

}

#endif //_CONCURRENCY_H_
