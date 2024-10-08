cmake_minimum_required(VERSION 3.28)
project(Compiler)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_executable(Compiler
        lexer.cpp
        include/token.h
        main.cpp
        include/lexer.h
        include/configure.h
        include/error.h
        include/parser.h
        parser.cpp
        include/ast.h
        ast.cpp)
