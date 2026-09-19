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

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE SYSTEM
    esp32/mocks
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    .
    lib
    fpga_cores

    esp32
    esp32/FpgaCores
    esp32/HID
    esp32/VFS
    esp32/USB

    external/cjson
    external/imgui
    external/libz80
    external/lodepng
    external/tinyfiledialogs

    external/xz
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Audio.cpp
    Config.cpp
    main.cpp
    MemoryEditor.cpp
    UI.cpp
    Version.cpp
    Midi.cpp
    
    lib/Common.cpp
    
    fpga_cores/EmuState.cpp
    fpga_cores/Z80Core.cpp
    fpga_cores/AssemblyListing.cpp
    fpga_cores/AY8910.cpp
    fpga_cores/SN76489.cpp

    fpga_cores/aqplus/AqpEmuState.cpp
    fpga_cores/aqplus/AqpVideo.cpp

    fpga_cores/aqms/AqmsEmuState.cpp
    fpga_cores/aqms/VDP.cpp

    fpga_cores/aq32/Aq32EmuState.cpp
    fpga_cores/aq32/Aq32Video.cpp
    fpga_cores/aq32/Aq32FmSynth.cpp
    fpga_cores/aq32/Aq32Pcm.cpp
    fpga_cores/aq32/cpu/riscv.cpp
    fpga_cores/aq32/cpu/riscv_util.cpp

    fpga_cores/aqua-8/Aqua8EmuState.cpp

    esp32/main.cpp
    esp32/FPGA.cpp
    esp32/Keyboard.cpp
    esp32/UartProtocol.cpp
    esp32/MidiData.cpp
    esp32/xz.c
    esp32/VFS/VFS.cpp
    esp32/VFS/EspVFS.cpp
    esp32/VFS/HttpVFS.cpp
    esp32/VFS/SDCardVFS.cpp
    esp32/VFS/TcpVFS.cpp
    esp32/FpgaCores/AquariusPlus/CoreAquariusPlus.cpp
    esp32/FpgaCores/Aquarius32/CoreAquarius32.cpp
    esp32/FpgaCores/FpgaCore.cpp
    esp32/FpgaCores/KbHcEmu.cpp
    esp32/DisplayOverlay/DisplayOverlay.cpp
    esp32/DisplayOverlay/Menu.cpp
    esp32/DisplayOverlay/Menus.cpp
    esp32/mocks/FreeRtosMock.cpp
    esp32/mocks/nvs_flash.cpp
    esp32/mocks/esp_ota_ops.cpp
    esp32/USB/USBHost.cpp

    external/cjson/cJSON.c
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_impl_sdl2.cpp
    external/imgui/imgui_impl_sdlrenderer2.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui.cpp
    external/libz80/z80.c
    external/lodepng/lodepng.cpp
    external/tinyfiledialogs/tinyfiledialogs.cpp
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE EMULATOR)

if (LINUX)
    find_package(PkgConfig)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wno-psabi -Wno-unused-parameter)

    # Add SDL includes/library/options
    find_package(SDL2 REQUIRED)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE ${SDL2_CFLAGS_OTHER})

    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE asound)
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
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W3)
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        windows
        windows/SDL2-2.0.20/include
    )
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE windows/Resource.rc)
    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE /SUBSYSTEM:WINDOWS)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/windows/SDL2-2.0.20/lib/x64/SDL2.lib)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/windows/SDL2-2.0.20/lib/x64/SDL2main.lib)
endif()
