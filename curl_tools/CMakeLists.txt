cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11")

project(curl_tools)
include_directories(.)
add_library(curl_tools STATIC tools.cpp session_factory.cpp)
