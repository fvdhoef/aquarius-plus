cmake_minimum_required(VERSION 3.13)

project(midiplay VERSION 0.1 LANGUAGES C CXX ASM)

include(../lib/project.cmake)

target_sources(${PROJECT_NAME} PRIVATE
    main.c
    fmsynth.c
    playfile.c
)
