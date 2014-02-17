cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11")

project(logger)
include_directories(.)
add_library(logger STATIC logger.cpp)
