cmake_minimum_required(VERSION 3.13)

project(aq32basic VERSION 0.1 LANGUAGES C CXX ASM)

add_executable(${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wshadow -Winit-self -Wfloat-conversion -Wdouble-promotion -Wmissing-include-dirs -Wlogical-op)
target_compile_options(${PROJECT_NAME} PRIVATE -O0 -g)

target_compile_definitions(${PROJECT_NAME} PRIVATE PCDEV)

target_include_directories(${PROJECT_NAME} PRIVATE
    .
    ../../lib
    ../basic
    ..
)

target_sources(${PROJECT_NAME} PRIVATE
    main.c
    console.c
    buffers.c

    ../../lib/common.c
    ../../lib/readline.c

    ../editor/editbuf.c

    ../basic/basic.c
    
    # ../basic/common/buffers.c
    ../basic/common/parsenum.c

    ../basic/parser/reloc.c
    ../basic/parser/tokenizer.c
    ../basic/parser/parser.c

    ../basic/bytecode/bytecode.c
    ../basic/bytecode/bytecode_variables.c
    ../basic/bytecode/bytecode_dump.c
    ../basic/bytecode/bytecode_math.c
    ../basic/bytecode/bytecode_string.c
    ../basic/bytecode/bytecode_console_io.c
    ../basic/bytecode/bytecode_file_io.c
)

target_link_libraries(${PROJECT_NAME} PRIVATE m)
