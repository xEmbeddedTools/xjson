cmake_minimum_required(VERSION 3.21)
project("xjson tests")

add_executable(tests "main.cpp")

add_subdirectory(externals/catch2)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/.." "${CMAKE_CURRENT_LIST_DIR}/../out/tests")
target_link_libraries(tests PRIVATE xjson Catch2)