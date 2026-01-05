project(${PROJECT_NAME}-server)

add_executable(${PROJECT_NAME} main.cc utils/Utils.cc utils/Json.cc)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CORE_TARGET})

# Dependencies
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# sodium
message(STATUS "Fetching dependency: sodium")
FetchContent_Declare(
  Sodium
  GIT_REPOSITORY https://github.com/robinlinden/libsodium-cmake.git
  GIT_TAG e5b985ad0dd235d8c4307ea3a385b45e76c74c6a)
set(SODIUM_DISABLE_TESTS ON)
FetchContent_MakeAvailable(Sodium)
target_link_libraries(${PROJECT_NAME} PRIVATE sodium)

# drogon
add_subdirectory(thirdparty/drogon)
target_link_libraries(${PROJECT_NAME} PRIVATE drogon)

aux_source_directory(controllers CTL_SRC)
aux_source_directory(filters FILTER_SRC)
aux_source_directory(plugins PLUGIN_SRC)
aux_source_directory(models MODEL_SRC)

drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
                    ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(
  ${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                          ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_sources(${PROJECT_NAME} PRIVATE ${SRC_DIR} ${CTL_SRC} ${FILTER_SRC}
                                       ${PLUGIN_SRC} ${MODEL_SRC})

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.json"
               "${CMAKE_CURRENT_BINARY_DIR}/config.json" COPYONLY)
