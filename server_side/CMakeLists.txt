cmake_minimum_required(VERSION 3.15)

set(SERVICE_NAME com.system.configurationManager)

project(${SERVICE_NAME})

find_package(sdbus-c++ REQUIRED)
add_subdirectory(lib/nlohmann_json)

add_executable(${SERVICE_NAME} src/main.cpp src/object_connection.cpp)
target_link_libraries(${SERVICE_NAME} PRIVATE SDBusCpp::sdbus-c++)
target_link_libraries(${SERVICE_NAME} PRIVATE nlohmann_json::nlohmann_json)

target_compile_options(${SERVICE_NAME} PRIVATE -std=c++17 -Wall -Wextra -Wpedantic)

install(DIRECTORY DESTINATION $ENV{HOME}/${SERVICE_NAME})
