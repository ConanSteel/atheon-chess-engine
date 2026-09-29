cmake_minimum_required(VERSION 3.14)
project(ChessEngine VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(chess_engine 
    src/main.cpp 
    src/bitboard.cpp
    src/board.cpp
    src/attacks.cpp
    src/movegen.cpp
    src/perft.cpp
    src/uci.cpp
    src/eval.cpp
    src/search.cpp
    src/zobrist.cpp
    src/tt.cpp
)

add_executable(chess_tests
    tests/test_main.cpp
    #tests/test_board.cpp
    #tests/test_attacks.cpp
    #tests/test_movegen.cpp
    tests/test_perft.cpp
    #tests/test_eval.cpp
    #tests/test_search.cpp
    #tests/test_zobrist.cpp
    
    src/bitboard.cpp
    src/board.cpp
    src/attacks.cpp
    src/movegen.cpp
    src/perft.cpp
    src/uci.cpp
    src/eval.cpp
    src/search.cpp
    src/zobrist.cpp
    src/tt.cpp
)

target_include_directories(chess_engine PRIVATE src)
target_include_directories(chess_tests PRIVATE src)

if(MSVC)
    target_compile_options(chess_engine PRIVATE /W4)
else()
    target_compile_options(chess_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)

target_link_libraries(chess_tests PRIVATE Catch2::Catch2)
enable_testing()
add_test(NAME ChessTests COMMAND chess_tests)
