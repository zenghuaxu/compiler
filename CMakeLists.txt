cmake_minimum_required(VERSION 3.28)
project(Compiler)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_executable(Compiler
        libs/frontend/src/lexer.cpp
        libs/frontend/include/token.h
        libs/main.cpp
        libs/frontend/include/lexer.h
        libs/include/configure.h
        libs/frontend/include/error.h
        libs/frontend/include/parser.h
        libs/frontend/src/parser.cpp
        libs/frontend/include/ast.h
        libs/frontend/src/ast.cpp
        libs/frontend/include/visitor.h
        libs/frontend/include/symtable.h
        libs/frontend/src/visitor.cpp
        libs/include/io.h)
