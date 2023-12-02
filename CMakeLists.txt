cmake_minimum_required(VERSION 3.0.0)

project(mysqlconnpoll C CXX)

set(CMAKE_CXX_STANDARD 11)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/poll)

add_subdirectory(${PROJECT_SOURCE_DIR}/poll)

link_directories(${PROJECT_SOURCE_DIR}/poll)

link_libraries(mysqlconnpoll pthread jsoncpp mysqlclient)

add_executable(test.a test.cpp)