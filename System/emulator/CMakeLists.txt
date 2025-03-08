cmake_minimum_required(VERSION 3.15)

project(aqplus-emu
    VERSION 1.2
    DESCRIPTION "Emulator for the Aquarius+ system"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${CMAKE_PROJECT_NAME})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    .
    imgui
    lodepng
    UI
    VFS
    libz80
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    AqKeyboard.cpp
    AqUartProtocol.cpp
    Audio.cpp
    AY8910.cpp
    Config.cpp
    EmuState.cpp
    main.cpp
    tinyfiledialogs.cpp
    Version.cpp
    Video.cpp

    cJSON.c

    UI/AssemblyListing.cpp
    UI/MemoryEditor.cpp
    UI/UI.cpp

    VFS/EspVFS.cpp
    VFS/HttpVFS.cpp
    VFS/SDCardVFS.cpp
    VFS/TcpVFS.cpp

    libz80/z80.c

    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_impl_sdl2.cpp
    imgui/imgui_impl_sdlrenderer2.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp

    lodepng/lodepng.cpp


)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wno-psabi -Wno-unused-parameter)

find_package(PkgConfig)

# Add SDL includes/library/options
find_package(SDL2 REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} ${SDL2_LIBRARIES})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE ${SDL2_CFLAGS_OTHER})
