cmake_minimum_required(VERSION 3.13)

execute_process(COMMAND
    git describe --always
    OUTPUT_VARIABLE APP_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

project(Aquarius32Emulator)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(
    .
    UI
    CPU
    Peripherals
    External/imgui
    External/tinyfiledialogs
    External/cJSON
)

add_executable(${CMAKE_PROJECT_NAME})

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Config.cpp
    EmuState.cpp
    main.cpp
    Version.cpp

    DebugInterface.cpp

    Assets/font.c

    UI/MemoryEditor.cpp
    UI/UI.cpp

    CPU/riscv.cpp
    CPU/riscv_util.cpp

    Peripherals/Video.cpp
    
    External/imgui/imgui.cpp
    External/imgui/imgui_draw.cpp
    External/imgui/imgui_impl_sdlrenderer2.cpp
    External/imgui/imgui_widgets.cpp
    External/imgui/imgui_demo.cpp
    External/imgui/imgui_impl_sdl2.cpp
    External/imgui/imgui_tables.cpp

    External/tinyfiledialogs/tinyfiledialogs.cpp

    External/cJSON/cJSON.c
)

# Set compiler options
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wno-psabi -Wall -Wextra -Wpedantic)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -O3 -g)

# pkg-config
find_package(PkgConfig)

# Add SDL includes/library/options
find_package(SDL2 REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} ${SDL2_LIBRARIES})
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC ${SDL2_CFLAGS_OTHER})
