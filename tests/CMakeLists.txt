# GoogleTest: сначала ищем системный пакет (libgtest-dev),
# иначе скачиваем из исходников — чтобы сборка работала и на чистой машине, и в CI.
find_package(GTest QUIET)

if(NOT GTest_FOUND)
  message(STATUS "GTest не найден в системе — подтягиваю через FetchContent")
  include(FetchContent)
  FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
    GIT_SHALLOW TRUE
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()

set(TEST_SOURCES
  smoke_test.cpp
  storage_test.cpp
)

add_executable(server_tests ${TEST_SOURCES} ${SERVER_LIB_SOURCES})
target_include_directories(server_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_options(server_tests PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(server_tests PRIVATE
  GTest::gtest
  GTest::gtest_main
  Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(server_tests)
