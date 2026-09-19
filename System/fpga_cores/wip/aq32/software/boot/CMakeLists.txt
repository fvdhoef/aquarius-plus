cmake_minimum_required(VERSION 3.13)

project(boot VERSION 0.1 LANGUAGES C CXX ASM)

include(../lib/project.cmake)

target_compile_definitions(${PROJECT_NAME} PRIVATE NO_LIB_INIT)

target_sources(${PROJECT_NAME} PRIVATE
    main.c
)
