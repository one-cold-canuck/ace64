cmake_minimum_required(VERSION 3.14)

project(ace64 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

find_package(Threads REQUIRED)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set (ace64_sources
  "code/cpu.h"
  "code/cpu.c"
  "code/opcodes.h"
  "code/opcodes.c"
  "test/ace64_test.cpp"
)

source_group("src" FILES ${ace64_sources})

add_executable(ace64_test ${ace64_sources})
target_link_libraries(
  ace64_test 
  GTest::gtest_main
  GTest::gtest
${CMAKE_THREAD_LIBS_INIT})

include(GoogleTest)
gtest_discover_tests(ace64_test)
