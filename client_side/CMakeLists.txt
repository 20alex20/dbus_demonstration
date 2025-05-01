cmake_minimum_required(VERSION 3.15)

set(APPLICATION_NAME confManagerApplication1)
set(SERVICE_NAME com.system.configurationManager)

project(${APPLICATION_NAME})

find_package(sdbus-c++ REQUIRED)
add_subdirectory(lib/nlohmann_json)

add_executable(${APPLICATION_NAME} src/main.cpp src/proxy_connection.cpp)
target_link_libraries(${APPLICATION_NAME} PRIVATE SDBusCpp::sdbus-c++)
target_link_libraries(${APPLICATION_NAME} PRIVATE nlohmann_json::nlohmann_json)

target_compile_options(${APPLICATION_NAME} PRIVATE -std=c++17 -Wall -Wextra -Wpedantic)

install(FILES install/configFile.json DESTINATION $ENV{HOME}/${SERVICE_NAME} RENAME ${APPLICATION_NAME}.json)
