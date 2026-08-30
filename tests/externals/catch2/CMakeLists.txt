cmake_minimum_required(VERSION 3.21)

add_library(Catch2 "${CMAKE_CURRENT_LIST_DIR}/catch.cpp")
target_compile_features(Catch2 PUBLIC cxx_std_20)
target_compile_definitions(Catch2 PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN)
target_include_directories(Catch2 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../")
