# https://github.com/azure-rtos/getting-started/blob/azrtos_v6.2.0/cmake/renesas-rx-gcc-toolchain.cmake
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
# Modified by NoMaY for RL78 SmartConfigurator examples.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR rl78) # Probably it makes no sense to do this.
set(TARGET_TRIPLET "rl78")

# default to Debug build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release." FORCE)
endif()

# do some windows specific logic
if(WIN32)
    set(TOOLCHAIN_EXT ".exe")
else()
    set(TOOLCHAIN_EXT "")
endif(WIN32)

find_program(COMPILER_ON_PATH "clang${TOOLCHAIN_EXT}")

if(DEFINED ENV{RL78_CLANG_PATH}) 
    # use the environment variable first    
    file(TO_CMAKE_PATH $ENV{RL78_CLANG_PATH} RL78_TOOLCHAIN_PATH)
    message(STATUS "Using ENV variable RL78_CLANG_PATH = ${RL78_TOOLCHAIN_PATH}")
elseif(COMPILER_ON_PATH) 
    # then check on the current path
    get_filename_component(RL78_TOOLCHAIN_PATH ${COMPILER_ON_PATH} DIRECTORY)
    message(STATUS "Using RL78 Clang from path = ${RL78_TOOLCHAIN_PATH}")
else()
    message(FATAL_ERROR "Unable to find Clang. Either add to your PATH, or define RL78_CLANG_PATH to the compiler location")
endif()

# Perform compiler test with the static library
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER    ${RL78_TOOLCHAIN_PATH}/clang${TOOLCHAIN_EXT} --target=${TARGET_TRIPLET} CACHE STRING "")
set(CMAKE_CXX_COMPILER  ${RL78_TOOLCHAIN_PATH}/clang++${TOOLCHAIN_EXT} --target=${TARGET_TRIPLET} CACHE STRING "")
set(CMAKE_ASM_COMPILER  ${RL78_TOOLCHAIN_PATH}/clang${TOOLCHAIN_EXT} --target=${TARGET_TRIPLET})
set(CMAKE_LINKER        ${RL78_TOOLCHAIN_PATH}/clang${TOOLCHAIN_EXT})
set(CMAKE_SIZE_UTIL     ${RL78_TOOLCHAIN_PATH}/llvm-size${TOOLCHAIN_EXT})
set(CMAKE_OBJCOPY       ${RL78_TOOLCHAIN_PATH}/rl78-elf-objcopy${TOOLCHAIN_EXT})
set(CMAKE_OBJDUMP       ${RL78_TOOLCHAIN_PATH}/llvm-objdump${TOOLCHAIN_EXT})
set(CMAKE_NM_UTIL       ${RL78_TOOLCHAIN_PATH}/llvm-nm${TOOLCHAIN_EXT})
set(CMAKE_AR            ${RL78_TOOLCHAIN_PATH}/llvm-ar${TOOLCHAIN_EXT})
set(CMAKE_RANLIB        ${RL78_TOOLCHAIN_PATH}/llvm-ranlib${TOOLCHAIN_EXT})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# <LLVM for Renesas RL78 Install Folder>/user_manual.pdf
# <LLVM for Renesas RL78 Install Folder>/Doc/Clang Compiler User's Manual Clang 10 documentation.pdf
# https://llvm-gcc-renesas.com/technical-faq/

set(MCPU_FLAGS "-mcpu=s3")
set(VFP_FLAGS  "")
set(CMAKE_COMMON_FLAGS "\
-ffunction-sections -fdata-sections \
-Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wpointer-arith -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wundef -Wno-unused-function -Wno-unused-label -Wno-main-return-type \
-g \
")
set(CMAKE_C_FLAGS   "${MCPU_FLAGS} ${VFP_FLAGS} ${CMAKE_COMMON_FLAGS} -gdwarf-2 -frenesas-extensions -U__CCRL__")
set(CMAKE_CXX_FLAGS "${MCPU_FLAGS} ${VFP_FLAGS} ${CMAKE_COMMON_FLAGS} -gdwarf-2 -frenesas-extensions -U__CCRL__")
set(CMAKE_ASM_FLAGS "${MCPU_FLAGS} ${VFP_FLAGS} ${CMAKE_COMMON_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${MCPU_FLAGS} ${VFP_FLAGS} ${CMAKE_COMMON_FLAGS} \
-Wl,--start-group -Wl,--end-group -nostartfiles -Wl,-e_PowerON_Reset -Wl,--gc-sections -Wl,--icf=none \
") # -Wl,--cref

if(RENESAS_INTELLISENSE_HELPER_EXTRA_FLAGS)
    string(APPEND CMAKE_C_FLAGS   " " ${RENESAS_INTELLISENSE_HELPER_EXTRA_FLAGS})
    string(APPEND CMAKE_CXX_FLAGS " " ${RENESAS_INTELLISENSE_HELPER_EXTRA_FLAGS})
endif()

set(CMAKE_C_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_ASM_FLAGS_DEBUG "-O0")
set(CMAKE_ASM_FLAGS_DEBUG "")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "")

set(CMAKE_C_FLAGS_RELEASE "-Os")
set(CMAKE_CXX_FLAGS_RELEASE "-Os")
set(CMAKE_ASM_FLAGS_RELEASE "")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")

# https://github.com/azure-rtos/getting-started/blob/azrtos_v6.2.0/cmake/utilities.cmake
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
# Modified by NoMaY for RL78 SmartConfigurator examples.

function(post_build TARGET)
    add_custom_target(${TARGET}.bin ALL 
        DEPENDS ${TARGET}
#        COMMAND ${CMAKE_OBJCOPY} -Obinary -R .dtc_vectortable -R .dtc_controldata_* -R .ocd_traceram ${TARGET}.elf ${TARGET}.bin
        COMMAND ${CMAKE_OBJCOPY} -Osrec -R .dtc_vectortable -R .dtc_controldata_* -R .ocd_traceram ${TARGET}.elf ${TARGET}.srec
        COMMAND ${CMAKE_OBJDUMP} -S ${TARGET}.elf > ${TARGET}.objdump 2> ${TARGET}.objdumperror
        COMMAND ${CMAKE_SIZE_UTIL} --format=berkeley ${TARGET}.elf
        COMMENT "Executing post build steps")
endfunction()

function(set_target_linker TARGET LINKER_SCRIPT)
    target_link_options(${TARGET} PRIVATE -T${LINKER_SCRIPT})
    target_link_options(${TARGET} PRIVATE -Wl,-Map=${TARGET}.map)
    set_target_properties(${TARGET} PROPERTIES SUFFIX ".elf") 
endfunction()

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()
