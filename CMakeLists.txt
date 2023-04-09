cmake_minimum_required(VERSION 3.22)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 23)

add_executable(ChessEngine main.cpp NetworkHandler.cpp NetworkHandler.h Chess.cpp Chess.h pch.h Board.cpp Board.h Player.h Move.h Engine.cpp Engine.h Timer.h Timer.cpp StaticEvaluator.cpp StaticEvaluator.h)
target_link_libraries(ChessEngine curl curlpp)
target_precompile_headers(ChessEngine PUBLIC pch.h)

set(ARCH_FLAGS "-mf16c -mavx2 -mlzcnt -mbmi -mbmi2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARCH_FLAGS}")