cmake_minimum_required(VERSION 3.8)
project(Concurrency)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp thread_guard.h threadsafe_stack.h hierarchical_mutex.h blocking_queue.h spinlock_mutex.h)
add_executable(Concurrency ${SOURCE_FILES})