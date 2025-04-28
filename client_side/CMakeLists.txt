cmake_minimum_required(VERSION 3.15)

project(confManagerApplication1)

find_package(sdbus-c++ REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE SDBusCpp::sdbus-c++)

