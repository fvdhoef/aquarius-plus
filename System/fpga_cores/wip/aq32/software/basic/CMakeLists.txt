cmake_minimum_required(VERSION 3.13)

project(basic VERSION 0.1 LANGUAGES C CXX ASM)

set(PRINTF_FLOAT ON)
set(MATHLIB on)
include(../lib/project.cmake)

target_compile_definitions(${PROJECT_NAME} PRIVATE SIZE_BUF_HEAP=0x1000)

target_include_directories(${PROJECT_NAME} PRIVATE
    basic
)

target_sources(${PROJECT_NAME} PRIVATE
    main.c
    video_save.c

    editor/cmd_edit.c
    editor/cmd_file.c
    editor/ctrl_edit_field.c
    editor/ctrl_file_list.c
    editor/dialog.c
    editor/editbuf.c
    editor/editor.c
    editor/menu.c
    editor/screen.c

    help/help.c

    basic/basic.c

    basic/common/buffers.c
    basic/common/parsenum.c

    basic/parser/reloc.c
    basic/parser/tokenizer.c
    basic/parser/parser.c

    basic/bytecode/bytecode.c
    basic/bytecode/bytecode_variables.c
    basic/bytecode/bytecode_math.c
    basic/bytecode/bytecode_string.c
    basic/bytecode/bytecode_console_io.c
    basic/bytecode/bytecode_file_io.c
)
