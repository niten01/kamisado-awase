project(${CORE_TARGET})

set(SOURCES src/Board.cpp src/Evaluator.cpp src/GameState.cpp src/MoveGen.cpp
            src/SearchEngine.cpp src/GameService.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC include)

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
