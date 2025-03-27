cmake_minimum_required(VERSION 3.13)

project(bootrom VERSION 0.1 LANGUAGES C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXECNAME bootrom)

set(APPLE FALSE)
set(CMAKE_C_COMPILER riscv64-elf-gcc)
set(CMAKE_CXX_COMPILER riscv64-elf-g++)
set(CMAKE_ASM_COMPILER riscv64-elf-gcc)
set(CMAKE_C_STANDARD 17)

# Get git version
execute_process(
    COMMAND git describe --tags --always
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE BUILD_ID
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

##############################################################################
# Project files
##############################################################################
add_executable(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${EXECNAME}.elf")

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND riscv64-elf-objcopy -O binary ${EXECNAME}.elf ${EXECNAME}.bin
    COMMAND riscv64-elf-objdump -d ${EXECNAME}.elf > dump.txt
    COMMAND riscv64-elf-objdump -x ${EXECNAME}.elf > dump_x.txt
    COMMAND riscv64-elf-objdump -t ${EXECNAME}.elf | expand -t36 | tail -n +5 | sort > dump_symbols.txt
    COMMAND riscv64-elf-size -A ${EXECNAME}.elf
    COMMAND riscv64-elf-size -G ${EXECNAME}.elf
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/genrom.py ${EXECNAME}.bin ${CMAKE_CURRENT_SOURCE_DIR}/../../fpga/src/bootrom.v
)

target_compile_options(${PROJECT_NAME} PRIVATE -Os -g)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wshadow -Winit-self -Wfloat-conversion -Wdouble-promotion -Wmissing-include-dirs -Wlogical-op)
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-unused-parameter -Wno-shadow)
target_compile_options(${PROJECT_NAME} PRIVATE -fdata-sections -ffunction-sections)
target_compile_options(${PROJECT_NAME} PRIVATE -march=rv32ima_zicsr_zicbom -mabi=ilp32 -ffreestanding)

target_link_options(${PROJECT_NAME} PRIVATE -march=rv32ima_zicsr -mabi=ilp32)
target_link_options(${PROJECT_NAME} PRIVATE -nostdlib)
target_link_options(${PROJECT_NAME} PRIVATE -Wl,--gc-sections)
target_link_options(${PROJECT_NAME} PRIVATE -Wl,--print-memory-usage)
target_link_options(${PROJECT_NAME} PRIVATE -Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/default.ld)

target_link_directories(${PROJECT_NAME} PRIVATE /usr/lib/gcc/riscv64-elf/14.1.0/rv32im/ilp32)
target_link_libraries(${PROJECT_NAME} gcc)

target_include_directories(${PROJECT_NAME} PRIVATE
    .
)

target_sources(${PROJECT_NAME} PRIVATE
    start.S
    boot.c
)
