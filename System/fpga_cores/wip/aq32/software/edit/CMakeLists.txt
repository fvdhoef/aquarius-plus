cmake_minimum_required(VERSION 3.13)

project(edit VERSION 0.1 LANGUAGES C CXX ASM)
include(../lib/project.cmake)

target_include_directories(${PROJECT_NAME} PRIVATE
    ../basic
)

target_sources(${PROJECT_NAME} PRIVATE
    main.c
    
    ../basic/video_save.c

    ../basic/editor/cmd_edit.c
    ../basic/editor/cmd_file.c
    ../basic/editor/ctrl_edit_field.c
    ../basic/editor/ctrl_file_list.c
    ../basic/editor/dialog.c
    ../basic/editor/editbuf.c
    ../basic/editor/editor.c
    ../basic/editor/menu.c
    ../basic/editor/screen.c
)
