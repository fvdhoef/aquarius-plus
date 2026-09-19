cmake_minimum_required(VERSION 3.13)

project(tiledemo VERSION 0.1 LANGUAGES C CXX ASM)

include(../lib/project.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    main.c
)
