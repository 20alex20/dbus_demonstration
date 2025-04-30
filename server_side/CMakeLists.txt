cmake_minimum_required(VERSION 3.15)

project(com.system.configurationManager)

find_package(sdbus-c++ REQUIRED)
add_subdirectory(lib/nlohmann_json)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE SDBusCpp::sdbus-c++)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

target_compile_options(${PROJECT_NAME} PRIVATE -std=c++17 -Wall -Wextra -Wpedantic)

install(DIRECTORY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/install/home_user/com.system.configurationManager)
