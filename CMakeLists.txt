cmake_minimum_required(VERSION 3.22)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 23)

add_executable(ChessEngine main.cpp NetworkHandler.cpp NetworkHandler.h Chess.cpp Chess.h pch.h)
target_link_libraries(ChessEngine curl curlpp)
target_precompile_headers(ChessEngine PUBLIC pch.h)
