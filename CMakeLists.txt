cmake_minimum_required(VERSION 3.14)
project(chess VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Create the shared library for main code
add_library(chesslib
    board.cpp
    engine.cpp
    move.cpp
    ANSIEsc.h    
    bitboard.h
    board.h
    chess.h
    chesstypes.h
    engine.h
    fen.h
    move.h
    square.h
)

target_include_directories(chesslib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}  # for .h files in the root directory
)

# Executable that links to the library
add_executable(chess main.cpp)
target_link_libraries(chess PRIVATE chesslib)

# Add unittests directory
add_subdirectory(unittests)
