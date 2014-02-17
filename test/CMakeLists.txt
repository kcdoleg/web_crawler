cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11")

project (test)
include_directories (.)
add_executable (main main.cpp)
target_link_libraries (main crawler crawler_exception curl_tools curl logger crypto)
