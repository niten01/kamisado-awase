project(${PROJECT_NAME}-engine-gui)

add_executable(${PROJECT_NAME} src/Main.cpp src/Game.cpp)

include_directories(include)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CORE_TARGET})

# Dependencies
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

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

# imgui
message(STATUS "Fetching dependency: imgui")
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG v1.92.5
  GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(imgui)
add_library(
  imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp)
target_include_directories(imgui INTERFACE ${imgui_SOURCE_DIR})

# rlimgui
message(STATUS "Fetching dependency: rlimgui")
FetchContent_Declare(
  rlimgui
  GIT_REPOSITORY https://github.com/raylib-extras/rlImGui
  GIT_TAG dc7f97679a024eee8f5f009e77cc311748200415)
FetchContent_MakeAvailable(rlimgui)
add_library(rlimgui STATIC ${rlimgui_SOURCE_DIR}/rlImGui.cpp)
target_link_libraries(rlimgui PUBLIC raylib imgui)
target_include_directories(rlimgui INTERFACE ${rlimgui_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE rlimgui)
