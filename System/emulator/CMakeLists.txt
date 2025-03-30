cmake_minimum_required(VERSION 3.15)

project(aqplus-emu
    VERSION 1.2
    DESCRIPTION "Emulator for the Aquarius+ system"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "" FORCE)
endif()

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

if (LINUX)
    find_package(PkgConfig)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wno-psabi -Wno-unused-parameter)

    # Add SDL includes/library/options
    find_package(SDL2 REQUIRED)
    target_link_libraries(${CMAKE_PROJECT_NAME} ${SDL2_LIBRARIES})
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE ${SDL2_CFLAGS_OTHER})
endif()

if (APPLE)
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/macos/AquariusPlusEmu.app/Contents/Frameworks/SDL2.framework)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -F${CMAKE_SOURCE_DIR}/macos/AquariusPlusEmu.app/Contents/Frameworks)
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/macos/AquariusPlusEmu.app/Contents/Frameworks/SDL2.framework/Headers)
    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -F${CMAKE_CURRENT_SOURCE_DIR}/macos/AquariusPlusEmu.app/Contents/Frameworks -framework SDL2 -Wl,-rpath,@executable_path/../Frameworks/)

    add_custom_command(
        TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND rm -rf ${CMAKE_BINARY_DIR}/AquariusPlusEmu.app
        COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/macos/AquariusPlusEmu.app ${CMAKE_BINARY_DIR}
        COMMAND mkdir -p ${CMAKE_BINARY_DIR}/AquariusPlusEmu.app/Contents/MacOS/
        COMMAND cp ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME} ${CMAKE_BINARY_DIR}/AquariusPlusEmu.app/Contents/MacOS/aquarius_emu
    )
endif()

if (MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W3 /WX)
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        windows
        windows/SDL2-2.0.20/include
    )
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE windows/Resource.rc)
    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE /SUBSYSTEM:WINDOWS)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/windows/SDL2-2.0.20/lib/x64/SDL2.lib)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/windows/SDL2-2.0.20/lib/x64/SDL2main.lib)
endif()
