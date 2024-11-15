cmake_minimum_required(VERSION 3.15)
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
        libs/include/io.h
        libs/llvm/include/ir/value.h
        libs/llvm/include/ir/use.h
        libs/llvm/include/ir/constant.h
        libs/llvm/include/llvmContext.h
        libs/llvm/include/ir/module.h
        libs/llvm/include/valueReturnType.h
        libs/llvm/include/valueReturnType.h
        libs/llvm/include/valueReturnType.h
        libs/llvm/include/llvm.h
        libs/llvm/include/ir/user.h
        libs/llvm/include/ir/instructions.h
        libs/llvm/include/ir/basicBlock.h
        libs/llvm/include/ir/function.h
        libs/llvm/include/ir/globalValue.h
        libs/llvm/include/ir/argument.h
        libs/llvm/include/ir/module.h
        libs/llvm/src/ir/instructions.cpp
        libs/llvm/src/ir/function.cpp
        libs/llvm/src/ir/globalValue.cpp
        libs/llvm/src/ir/basicBlock.cpp
        libs/llvm/src/ir/use.cpp
        libs/llvm/src/llvmContext.cpp
)
