cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

project(SerialConsole_getchar C CXX ASM)

add_executable(${PROJECT_NAME}
    src/smc_integration/RenesasSimDebug/sim_debug_mode_hook.c
    src/smc_integration/RenesasSimDebug/sim_debugger_console.c
    src/smc_integration/IDE_Helper/ide_helper.c
    src/smc_integration/r_cg_inthandler.c
    src/smc_integration/r_main.c
    src/smc_integration/start.S
    src/smc_integration/syscall.c
    src/smc_gen/r_bsp/mcu/rl78_g23/mcu_clocks.c
    src/smc_gen/r_bsp/mcu/all/r_bsp_common.c
    src/smc_gen/r_bsp/board/generic_rl78_g23/hdwinit.c
    src/smc_gen/r_bsp/board/generic_rl78_g23/r_bsp_init.c
    src/smc_gen/general/r_cg_itl_common.c
    src/smc_gen/general/r_cg_sau_common.c
    src/smc_gen/general/r_cg_systeminit.c
    src/smc_gen/general/r_cg_vect_table.c
    src/smc_gen/Config_UART0/Config_UART0.c
    src/smc_gen/Config_UART0/Config_UART0_user.c
    src/smc_gen/Config_PORT/Config_PORT.c
    src/smc_gen/Config_PORT/Config_PORT_user.c
    src/frtos_startup/freertos_helper.c
    src/frtos_startup/freertos_object_init.c
    src/frtos_startup/freertos_start.c
    src/FreeRTOS/Source/portable/Renesas/LLVMRL78/port.c
    src/FreeRTOS/Source/portable/Renesas/LLVMRL78/portasm.S
    src/FreeRTOS/Source/portable/MemMang/heap_4.c
    src/FreeRTOS/Source/event_groups.c
    src/FreeRTOS/Source/list.c
    src/FreeRTOS/Source/queue.c
    src/FreeRTOS/Source/stream_buffer.c
    src/FreeRTOS/Source/tasks.c
    src/FreeRTOS/Source/timers.c
    src/rl78g23_fpb_main.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    src/FreeRTOS/Source/portable/Renesas/LLVMRL78
    src/FreeRTOS/Source/include
    src/frtos_skeleton
    src/frtos_startup
    src/frtos_config
    src
    src/smc_integration
    src/smc_gen/Config_PORT
    src/smc_gen/Config_ITL0
    src/smc_gen/Config_UART0
    src/smc_gen/r_bsp
    src/smc_gen/r_config
    src/smc_gen/general
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    RL78_G23_FPB
    RENESAS_SIMULATOR_DEBUGGING=1
)

target_compile_features(${PROJECT_NAME}
    PRIVATE
    c_std_99
    cxx_std_17
)

set_target_linker(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src/linker_script.ld)

post_build(${PROJECT_NAME})
