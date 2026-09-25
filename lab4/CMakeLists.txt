cmake_minimum_required(VERSION 3.14)
project(snake_lab4_tests CXX)

set(CMAKE_CXX_STANDARD 17)

# Download and configure GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# Windows: prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# MinGW compatibility with GTest
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0601 -DGTEST_HAS_POSIX_RE=0")
FetchContent_MakeAvailable(googletest)

enable_testing()

# Part B — tests (no seam, standalone class definitions)
add_executable(tests
    tests.cpp
)
target_link_libraries(tests GTest::gtest_main)
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/..)

# Coverage flags (Part C)
if (MSVC)
  # MSVC doesn't support gcov; coverage is documented in REPORT.md
else()
  target_compile_options(tests PRIVATE --coverage -O0 -g)
  target_link_options(tests PRIVATE --coverage)
endif()

include(GoogleTest)
gtest_discover_tests(tests)
