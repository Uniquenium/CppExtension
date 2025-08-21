cmake_minimum_required(VERSION 3.18)

project(UniDeskCppExt)

set (CMAKE_CXX_STANDARD 17)

# pybind11 importieren
add_subdirectory(pybind11)

# create bindings
pybind11_add_module(UDFrameless BUDFrameless.cpp)
target_link_libraries(UDFrameless PRIVATE unideskcppext pybind11::embed)
