cmake_minimum_required(VERSION 3.15)

project(confManagerApplication1)

find_package(sdbus-c++ REQUIRED)
add_subdirectory(lib/nlohmann_json)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE SDBusCpp::sdbus-c++)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

install(DIRECTORY install/com.system.configurationManager DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/install/home_user)
