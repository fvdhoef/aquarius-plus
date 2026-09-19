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
    COMMAND riscv-none-elf-objcopy -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.aq32
    COMMAND riscv-none-elf-objdump -d ${PROJECT_NAME}.elf > dump.txt
    COMMAND riscv-none-elf-objdump -x ${PROJECT_NAME}.elf > dump_x.txt
    COMMAND riscv-none-elf-objdump -t ${PROJECT_NAME}.elf | expand -t36 | tail -n +5 | sort > dump_symbols.txt
    COMMAND riscv-none-elf-size -A ${PROJECT_NAME}.elf
    # COMMAND riscv-none-elf-size -G ${PROJECT_NAME}.elf
)

target_compile_options(${PROJECT_NAME} PRIVATE -g)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wshadow -Winit-self -Wfloat-conversion -Wdouble-promotion -Wmissing-include-dirs -Wlogical-op)
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-unused-parameter -Wno-shadow)
target_compile_options(${PROJECT_NAME} PRIVATE -fdata-sections -ffunction-sections -fno-strict-aliasing)
target_compile_options(${PROJECT_NAME} PRIVATE -march=rv32im_zicsr -mabi=ilp32 -specs=nano.specs)

target_link_options(${PROJECT_NAME} PRIVATE -march=rv32im -mabi=ilp32 -specs=nano.specs)

if (PRINTF_FLOAT)
target_link_options(${PROJECT_NAME} PRIVATE -u _printf_float)
endif()

target_link_options(${PROJECT_NAME} PRIVATE -nostartfiles)
target_link_options(${PROJECT_NAME} PRIVATE -Wl,--gc-sections)
target_link_options(${PROJECT_NAME} PRIVATE -Wl,--print-memory-usage)
target_link_options(${PROJECT_NAME} PRIVATE -Wl,-T${CMAKE_CURRENT_LIST_DIR}/default.ld)

target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/start.S
    ${CMAKE_CURRENT_LIST_DIR}/common.c
    ${CMAKE_CURRENT_LIST_DIR}/console.c
    ${CMAKE_CURRENT_LIST_DIR}/esp.c
    ${CMAKE_CURRENT_LIST_DIR}/malloc.c
    ${CMAKE_CURRENT_LIST_DIR}/readline.c
    ${CMAKE_CURRENT_LIST_DIR}/syscalls.c
)

if (MATHLIB)
target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()


add_custom_target(debug
    COMMAND riscv-none-elf-gdb --tui -q ${PROJECT_NAME}.elf -ex "target remote localhost:2331" -ex "load"
    DEPENDS ${PROJECT_NAME}
    USES_TERMINAL
    VERBATIM
)
