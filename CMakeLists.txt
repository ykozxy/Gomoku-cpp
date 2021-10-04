cmake_minimum_required(VERSION 3.19)
project(Gomoku)

set(CMAKE_CXX_STANDARD 17)

add_executable(Gomoku main.cpp)
add_executable(LocalTest test.cpp MinimaxAI.cpp MinimaxAI.h Board.cpp Board.h constants.h)
#add_executable(test out.cpp)