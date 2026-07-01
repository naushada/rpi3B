cmake_minimum_required(VERSION 3.16)
project(bcm2837_baremetal C CXX ASM)

# Standalone cross build (configure with -DCMAKE_TOOLCHAIN_FILE=toolchain-aarch64.cmake).
# Produces kernel8.img: the freestanding runtime that makes inc/interrupt.hpp's
# AArch64 VBAR_EL1 model real (boot -> EL1 drop -> vectors -> timer IRQ ->
# IRQ::dispatch). See README.md.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(REPO ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Bare-metal codegen: no libc/startup, no exceptions/RTTI, integer regs only
# (exception stubs don't save FP/SIMD), fixed-address (non-PIC) image.
set(FS_COMMON "-ffreestanding -fno-pic -mgeneral-regs-only -mcpu=cortex-a53 -Wall -Wextra")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${FS_COMMON}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -mcpu=cortex-a53")
# INTERRUPT_BAREMETAL makes IRQ::install_vector_table emit `msr VBAR_EL1`.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FS_COMMON} -fno-exceptions -fno-rtti -fno-stack-protector -DINTERRUPT_BAREMETAL")

add_executable(kernel8.elf
    boot.S
    vectors.S
    kmain.cpp
    exceptions.cpp
    uart.cpp
    libc_min.cpp
    ${REPO}/src/interrupt/interrupt.cpp          # the modelled IRQ/dispatch code
)

target_include_directories(kernel8.elf PRIVATE
    ${REPO}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_options(kernel8.elf PRIVATE
    -nostdlib -no-pie -static
    -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
    -Wl,--build-id=none
)
set_target_properties(kernel8.elf PROPERTIES
    LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld)

# ELF -> raw image QEMU/GPU firmware load at 0x80000.
add_custom_command(TARGET kernel8.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:kernel8.elf>
            ${CMAKE_BINARY_DIR}/kernel8.img
    BYPRODUCTS ${CMAKE_BINARY_DIR}/kernel8.img
    COMMENT "objcopy kernel8.elf -> kernel8.img")
