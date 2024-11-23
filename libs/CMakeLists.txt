add_subdirectory(frontend)
add_subdirectory(llvm)
add_subdirectory(mips)

add_executable(Compiler
        main.cpp
        include/configure.h
        include/io.h
)

target_link_libraries(Compiler
        frontend
        llvm
        mips
)
