cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11")

project (crawler)
include_directories(.)

add_library(crawler STATIC crawler.cpp url_utils.cpp)

