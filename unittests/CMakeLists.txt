cmake_minimum_required(VERSION 3.14)
message(STATUS "Processing unit_test source")

set(CMAKE_CXX_STANDARD 20 CACHE STRING "v")
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)


include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        main  # Use the latest commit from the main branch
)

# Alternatively, use a stable release tag (replace with the latest one)
# GIT_TAG        release-1.14.0

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()

if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(unittest
  fen.cpp
  pawn.cpp
  rook.cpp
  utils.cpp
  utils.h
)

# Link to both GTest and chesslib
target_link_libraries(unittest
  GTest::gtest_main
  chesslib
)

# Inherit includes from chesslib
target_include_directories(unittest PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
)

include(GoogleTest)
gtest_discover_tests(unittest)
