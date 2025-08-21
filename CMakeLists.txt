cmake_minimum_required(VERSION 3.18)


project(UniDeskCppExt)
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
add_subdirectory(src)
add_subdirectory(bindings)