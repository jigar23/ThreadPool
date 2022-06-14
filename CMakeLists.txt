cmake_minimum_required(VERSION 3.20)
project(threadpool)

set(CMAKE_CXX_STANDARD 17)

add_executable(threadpool example.cpp ThreadPool.h)
