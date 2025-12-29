cmake_minimum_required(VERSION 3.22)
project(kamisado)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

set(SOURCES
    src/Main.cpp
    src/Game.cpp
    src/Board.cpp
    src/Evaluator.cpp
    src/GameState.cpp
    src/MoveGen.cpp
    src/SearchEngine.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

# Dependencies
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# fmt
message(STATUS "Fetching dependency: fmt")
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 12.1.0)
FetchContent_MakeAvailable(fmt)
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)

# raylib
message(STATUS "Fetching dependency: raylib")
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.5
  GIT_SHALLOW 1
  GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(raylib)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# raylib-cpp
message(STATUS "Fetching dependency: raylib-cpp")
FetchContent_Declare(
  raylib_cpp
  GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git
  GIT_TAG v5.5.0)
FetchContent_MakeAvailable(raylib_cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib_cpp)
