cmake_minimum_required(VERSION 3.0)
set(PRJ_NAME json_test)
project(${PRJ_NAME})
set(CMAKE_CXX_STANDARD 14)




# 第一种方式：链接动态库
# include_directories(${PROJECT_SOURCE_DIR}/3rd/cJSON/include)
# link_directories(${PROJECT_SOURCE_DIR}/3rd/cJSON/lib)
# add_executable(${PRJ_NAME} cJSON_test.c)
# target_link_libraries(${PRJ_NAME} PUBLIC cjson)

# 第二种方式：编译源码
include_directories(${PROJECT_SOURCE_DIR}/lib/cJSON/src)
add_library(LIB_CJSON SHARED ${PROJECT_SOURCE_DIR}/lib/cJSON/src/cJSON.c)
add_executable(${PRJ_NAME} cJSON_test.c)
target_link_libraries(${PRJ_NAME} PUBLIC LIB_CJSON)

