cmake_minimum_required(VERSION 3.13)

project(basic VERSION 0.1 LANGUAGES C CXX ASM)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    main.c
)
