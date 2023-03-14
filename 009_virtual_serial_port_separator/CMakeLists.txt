cmake_minimum_required(VERSION 3.0)
set(PRJ_NAME vsps)
project(${PRJ_NAME})

set(CMAKE_CXX_STANDARD 11)

add_executable(${PRJ_NAME} main.cpp)

target_link_libraries(${PRJ_NAME} pthread util)
