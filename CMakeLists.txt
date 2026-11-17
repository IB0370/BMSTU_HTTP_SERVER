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

# Общие исходники сервера (всё, кроме main.cpp).
# Их линкуют и исполняемый файл, и тесты — список ведётся в одном месте.
# Пример: src/storage/in_memory_storage.cpp
set(SERVER_LIB_SOURCES
)

add_executable(server ${SERVER_LIB_SOURCES} src/main.cpp)
target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(server PRIVATE Threads::Threads)
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
