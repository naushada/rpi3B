# CMake toolchain for the freestanding aarch64 kernel image. Uses the GNU
# aarch64-linux-gnu cross toolchain (present in the freestanding/Dockerfile
# builder image) in a bare-metal (-nostdlib/-ffreestanding) configuration.
set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(TRIPLE aarch64-linux-gnu)
set(CMAKE_C_COMPILER   ${TRIPLE}-gcc)
set(CMAKE_CXX_COMPILER ${TRIPLE}-g++)
set(CMAKE_ASM_COMPILER ${TRIPLE}-gcc)
set(CMAKE_OBJCOPY      ${TRIPLE}-objcopy CACHE FILEPATH "objcopy")

# Freestanding: don't try to link a full test program during compiler detection.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_COMPILER_WORKS   1)
set(CMAKE_CXX_COMPILER_WORKS 1)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
