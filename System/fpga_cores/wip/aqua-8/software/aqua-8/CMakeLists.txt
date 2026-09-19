cmake_minimum_required(VERSION 3.13)

project(aqua-8 VERSION 0.1.0 LANGUAGES C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(APPLE FALSE)
set(CMAKE_C_COMPILER riscv-none-elf-gcc)
set(CMAKE_CXX_COMPILER riscv-none-elf-g++)
set(CMAKE_ASM_COMPILER riscv-none-elf-gcc)
set(CMAKE_C_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE MinSizeRel)
endif()

add_executable(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.elf")

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND riscv-none-elf-objcopy -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMAND riscv-none-elf-objdump -d ${PROJECT_NAME}.elf > dump.txt
    COMMAND riscv-none-elf-objdump -x ${PROJECT_NAME}.elf > dump_x.txt
    COMMAND riscv-none-elf-objdump -t ${PROJECT_NAME}.elf | expand -t36 | tail -n +5 | sort > dump_symbols.txt
    COMMAND riscv-none-elf-size -A ${PROJECT_NAME}.elf
    # COMMAND riscv-none-elf-size -G ${PROJECT_NAME}.elf
)

target_compile_options(${PROJECT_NAME} PRIVATE -g)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wshadow -Winit-self -Wmissing-include-dirs -Wlogical-op) # -Wfloat-conversion -Wdouble-promotion
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-unused-parameter -Wno-shadow)
target_compile_options(${PROJECT_NAME} PRIVATE -fdata-sections -ffunction-sections -fno-strict-aliasing)
target_compile_options(${PROJECT_NAME} PRIVATE -march=rv32im_zicsr_zicbom -mabi=ilp32 -specs=nano.specs)

target_link_options(${PROJECT_NAME} PRIVATE -march=rv32im -mabi=ilp32 -specs=nano.specs)

# if (PRINTF_FLOAT)
target_link_options(${PROJECT_NAME} PRIVATE -u _printf_float)
# endif()

target_link_options(${PROJECT_NAME} PRIVATE -nostartfiles)
target_link_options(${PROJECT_NAME} PRIVATE -Wl,--gc-sections)
target_link_options(${PROJECT_NAME} PRIVATE -Wl,--print-memory-usage)
target_link_options(${PROJECT_NAME} PRIVATE -Wl,-T${CMAKE_CURRENT_LIST_DIR}/default.ld)

target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

target_link_libraries(${PROJECT_NAME} PRIVATE m)

add_custom_target(debug
    COMMAND riscv-none-elf-gdb --tui -q ${PROJECT_NAME}.elf -ex "target remote localhost:2331" -ex "load"
    DEPENDS ${PROJECT_NAME}
    USES_TERMINAL
    VERBATIM
)

target_include_directories(${PROJECT_NAME} PRIVATE
    .
    lib
    # lua_aqua8
    # lua-5.2.4/src
)

target_sources(${PROJECT_NAME} PRIVATE
    main.c

    state.c
    scr.c

    draw/draw.c

    scr_console/console.c
    scr_console/readline.c
    scr_console/commands.c

    scr_gfx/scr_sprite.c
    scr_gfx/scr_map.c

    scr_snd/scr_sfx.c
    scr_snd/scr_music.c

    scr_code/scr_code.c
    scr_code/editbuf.c
    scr_code/edit_ops.c
    scr_code/colorize.c

    lib/start.S
    lib/common.c
    lib/esp.c
    lib/malloc.c
    lib/syscalls.c

    aq8lua/aq8lua.cpp

    z8lua/lapi.cpp
    z8lua/lauxlib.cpp
    z8lua/lbaselib.cpp
    z8lua/lcode.cpp
    z8lua/lcorolib.cpp
    z8lua/lctype.cpp
    z8lua/ldblib.cpp
    z8lua/ldebug.cpp
    z8lua/ldo.cpp
    z8lua/ldump.cpp
    z8lua/lfunc.cpp
    z8lua/lgc.cpp
    z8lua/linit.cpp
    z8lua/llex.cpp
    z8lua/lmem.cpp
    z8lua/lobject.cpp
    z8lua/lopcodes.cpp
    z8lua/lparser.cpp
    z8lua/lpico8lib.cpp
    z8lua/lstate.cpp
    z8lua/lstring.cpp
    z8lua/lstrlib.cpp
    z8lua/ltable.cpp
    z8lua/ltablib.cpp
    z8lua/ltm.cpp
    # z8lua/lua.cpp
    z8lua/lundump.cpp
    z8lua/lvm.cpp
    z8lua/lzio.cpp

    # lua_aqua8/lua_aqua8.c
    # lua_aqua8/func_fixed.c
    # lua_aqua8/func_gfx.c

    # lua/mychanges.c
    # lua/myfuncs.c
)

# Suppress fall-through warnings for a few files
set_source_files_properties(z8lua/lgc.cpp PROPERTIES COMPILE_FLAGS "-std=c++17 -Wno-implicit-fallthrough")
set_source_files_properties(z8lua/llex.cpp PROPERTIES COMPILE_FLAGS "-std=c++17 -Wno-implicit-fallthrough")
set_source_files_properties(z8lua/lstrlib.cpp PROPERTIES COMPILE_FLAGS "-std=c++17 -Wno-implicit-fallthrough")
set_source_files_properties(z8lua/ltable.cpp PROPERTIES COMPILE_FLAGS "-std=c++17 -Wno-implicit-fallthrough")
set_source_files_properties(z8lua/lvm.cpp PROPERTIES COMPILE_FLAGS "-std=c++17 -Wno-implicit-fallthrough")
