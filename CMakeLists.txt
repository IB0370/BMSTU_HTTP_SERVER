cmake_minimum_required(VERSION 3.16)
project(http_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Threads REQUIRED)

add_executable(server src/main.cpp)
target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(server PRIVATE Threads::Threads)
