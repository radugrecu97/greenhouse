cmake_minimum_required(VERSION 3.7)
project(greenhouse)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

add_executable(greenhouse main.cpp HIH8120.cpp I2CDevice.cpp)