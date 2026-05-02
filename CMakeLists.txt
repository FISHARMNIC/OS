cmake_minimum_required(VERSION 3.24)

set(CROSS_COMPILE "i686-elf-" CACHE STRING "Cross compiler prefix (example: i686-elf-)")
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR i686)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_OSX_SYSROOT "")
set(CMAKE_OSX_ARCHITECTURES "")
set(CMAKE_EXE_LINKER_FLAGS_INIT "")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "")

find_program(CROSS_GCC NAMES ${CROSS_COMPILE}gcc)
if(NOT CROSS_GCC)
    message(FATAL_ERROR "Could not find ${CROSS_COMPILE}gcc. Install a cross compiler or set -DCROSS_COMPILE=.")
endif()

find_program(CROSS_OBJCOPY NAMES ${CROSS_COMPILE}objcopy)
if(NOT CROSS_OBJCOPY)
    message(FATAL_ERROR "Could not find ${CROSS_COMPILE}objcopy.")
endif()

set(CMAKE_C_COMPILER "${CROSS_GCC}")
set(CMAKE_ASM_COMPILER "${CROSS_GCC}")

project(MyOS LANGUAGES C ASM)

set(OS_NAME "MyOS")
set(KERNEL_TARGET "kernel")
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/boot/linker.ld")
set(KERNEL_OUT_DIR "${CMAKE_BINARY_DIR}/compiled")
set(ISO_ROOT "${CMAKE_BINARY_DIR}/isodir")
set(ISO_FILE "${KERNEL_OUT_DIR}/${OS_NAME}.iso")
set(FONT_OBJ "${CMAKE_BINARY_DIR}/FONT.F16.o")

add_custom_command(
    OUTPUT "${FONT_OBJ}"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/disk/sys/FONT.F16" "FONT.F16"
    COMMAND ${CROSS_OBJCOPY} -I binary -O elf32-i386 -B i386 "FONT.F16" "FONT.F16.o"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    DEPENDS "${CMAKE_SOURCE_DIR}/disk/sys/FONT.F16"
    COMMENT "Embedding disk/sys/FONT.F16 into kernel"
    VERBATIM
)

set_source_files_properties("${FONT_OBJ}" PROPERTIES GENERATED TRUE EXTERNAL_OBJECT TRUE)

file(GLOB_RECURSE SRC_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/libs/src/*.c"
)

add_executable(${KERNEL_TARGET}
    boot/boot.S
    kernel/main.c
    ${SRC_SOURCES}
    "${FONT_OBJ}"
)

target_include_directories(${KERNEL_TARGET} PRIVATE
    "${CMAKE_SOURCE_DIR}/libs/inc"
)

target_compile_options(${KERNEL_TARGET} PRIVATE
    -m32
    -ffreestanding
    -fno-pic
    -fno-pie
    -fno-stack-protector
    -Wall
    -Wextra
    -Werror
    -Wno-unused-parameter
)

target_link_options(${KERNEL_TARGET} PRIVATE
    -m32
    -no-pie
    -nostdlib
    -nostartfiles
    -nodefaultlibs
    -Wl,-T,${LINKER_SCRIPT}
    -Wl,--build-id=none
)

set_property(TARGET ${KERNEL_TARGET} APPEND PROPERTY LINK_DEPENDS "${LINKER_SCRIPT}")

set_target_properties(${KERNEL_TARGET} PROPERTIES
    OUTPUT_NAME "${OS_NAME}"
    SUFFIX ".bin"
    RUNTIME_OUTPUT_DIRECTORY "${KERNEL_OUT_DIR}"
)

find_program(GRUB_FILE NAMES grub-file)
if(GRUB_FILE)
    add_custom_command(TARGET ${KERNEL_TARGET} POST_BUILD
        COMMAND ${GRUB_FILE} --is-x86-multiboot $<TARGET_FILE:${KERNEL_TARGET}>
        COMMENT "Checking multiboot header"
        VERBATIM
    )
endif()

find_program(GRUB_MKRESCUE NAMES grub-mkrescue)
if(GRUB_MKRESCUE)
    add_custom_command(
        OUTPUT "${ISO_FILE}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ISO_ROOT}/boot/grub"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${KERNEL_TARGET}>" "${ISO_ROOT}/boot/${OS_NAME}.bin"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/boot/FONT.F16" "${ISO_ROOT}/boot/FONT.F16"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/boot/grub.cfg" "${ISO_ROOT}/boot/grub/grub.cfg"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${KERNEL_OUT_DIR}"
        COMMAND ${GRUB_MKRESCUE} -o "${ISO_FILE}" "${ISO_ROOT}"
        DEPENDS ${KERNEL_TARGET} "${CMAKE_SOURCE_DIR}/boot/grub.cfg" "${CMAKE_SOURCE_DIR}/boot/FONT.F16"
        COMMENT "Creating bootable ISO"
        VERBATIM
    )

    add_custom_target(iso DEPENDS "${ISO_FILE}")
else()
    message(STATUS "grub-mkrescue not found; ISO target disabled.")
endif()

find_program(QEMU_SYSTEM_I386 NAMES qemu-system-i386)
if(QEMU_SYSTEM_I386 AND TARGET iso)
    add_custom_target(run
        COMMAND ${QEMU_SYSTEM_I386} -machine pc -cdrom "${ISO_FILE}"
        DEPENDS iso
        COMMENT "Running OS in QEMU"
        USES_TERMINAL
    )
endif()
