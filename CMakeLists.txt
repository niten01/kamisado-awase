cmake_minimum_required(VERSION 3.22)
project(kamisado)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CORE_TARGET kamisado-core)

add_subdirectory(core)
add_subdirectory(tools)
