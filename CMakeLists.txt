cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11")

project (all)
include_directories(.)
add_subdirectory(logger)
add_subdirectory(crawler_exception)
add_subdirectory(curl_tools)
add_subdirectory(crawler)
add_subdirectory(test)

