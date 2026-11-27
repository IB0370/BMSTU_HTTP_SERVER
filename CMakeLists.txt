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

# Общий код сервера (всё, кроме main.cpp): компилируется один раз,
# линкуется и в server, и в тесты. Новые файлы добавляйте сюда.
add_library(core STATIC
  src/config/config.cpp
  # src/storage/global_mutex_storage.cpp
)
target_include_directories(core PUBLIC src)
target_link_libraries(core PUBLIC Threads::Threads)
target_compile_options(core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(server src/main.cpp)
target_link_libraries(server PRIVATE core)
target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
