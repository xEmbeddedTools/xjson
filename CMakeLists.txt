cmake_minimum_required(VERSION 3.21)
project(xjson LANGUAGES CXX)

add_library(xjson STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src/Document.cpp")
target_include_directories(xjson PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}\\include\\")
target_compile_features(xjson PUBLIC cxx_std_20)