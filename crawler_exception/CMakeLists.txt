cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "-std=c++11")

project (crawler_exception)
include_directories (.)
add_library (crawler_exception STATIC crawler_exception.cpp)
