cmake_minimum_required(VERSION 3.5)
project(redis_3_0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")

set(SOURCE_FILES src/redis.c)
add_executable(redis_3_0 ${SOURCE_FILES})