cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Select one exercise application while reusing the common STM32 board layer.
# Example: cmake --preset Debug -DEXERCISE=week1/ex2
set(DELAY_MODE 0 CACHE STRING "0=vTaskDelay, 1=xTaskDelayUntil")
set(APP_TICK_RATE_HZ 1000 CACHE STRING "FreeRTOS tick rate")
set(OVERHEAD_TEST 0 CACHE STRING "1=measure tick ISR overhead, 0=run periodic timing test")
set(ADD_EXTRA_STATIC_TASK 0 CACHE STRING "1=add the Week1/Ex4 trade-off task")
set(APP_USE_PREEMPTION 1 CACHE STRING "FreeRTOS preemptive scheduler: 0 or 1")
set(EX2_MODE 2 CACHE STRING "Week1/Ex2 experiment mode: 2, 3, 4, or 5")
set(EX2_C_WORK_MULTIPLIER 1 CACHE STRING "Week1/Ex2 task C work multiplier")
set(EXERCISE "week1/ex2" CACHE STRING "Exercise directory to build")
get_filename_component(EXERCISE_DIR "${EXERCISE}" ABSOLUTE
    BASE_DIR "${CMAKE_SOURCE_DIR}")

if(NOT IS_DIRECTORY "${EXERCISE_DIR}/app")
    message(FATAL_ERROR "Exercise app directory not found: ${EXERCISE_DIR}/app")
endif()

if(NOT EXISTS "${EXERCISE_DIR}/app/params.h")
    message(FATAL_ERROR
        "Missing ${EXERCISE_DIR}/app/params.h. "
        "Run: python tools/gen_params.py <MSSV>")
endif()

file(RELATIVE_PATH EXERCISE_REL "${CMAKE_SOURCE_DIR}" "${EXERCISE_DIR}")
string(REPLACE "/" "_" TARGET_NAME "${EXERCISE_REL}")
string(REPLACE "\\" "_" TARGET_NAME "${TARGET_NAME}")

project(${TARGET_NAME})
enable_language(C ASM)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Exercise: ${EXERCISE_REL}")
message(STATUS "Firmware: ${TARGET_NAME}.elf")

add_executable(${PROJECT_NAME})
target_compile_definitions(${PROJECT_NAME} PRIVATE
    DELAY_MODE=${DELAY_MODE}
    OVERHEAD_TEST=${OVERHEAD_TEST}
    ADD_EXTRA_STATIC_TASK=${ADD_EXTRA_STATIC_TASK}
    EX2_MODE=${EX2_MODE}
    EX2_C_WORK_MULTIPLIER=${EX2_C_WORK_MULTIPLIER}
)

# CubeMX-generated board support is shared by every exercise.
add_subdirectory(template/cmake/stm32cubemx)

# FreeRTOS port and heap used by the STM32F103 Cortex-M3 target.
set(FREERTOS_PORT "GCC_ARM_CM3" CACHE STRING "" FORCE)
set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)

add_library(freertos_config INTERFACE)
target_compile_definitions(freertos_config INTERFACE
    STM32F103xB
    USE_HAL_DRIVER
    APP_TICK_RATE_HZ=${APP_TICK_RATE_HZ}
    APP_USE_PREEMPTION=${APP_USE_PREEMPTION}
)
target_include_directories(freertos_config INTERFACE
    ${EXERCISE_DIR}/app
    ${CMAKE_SOURCE_DIR}/week1/ex1/app
    ${CMAKE_SOURCE_DIR}/template/Core/Inc
    ${CMAKE_SOURCE_DIR}/template/Drivers/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/template/Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${CMAKE_SOURCE_DIR}/template/Drivers/STM32F1xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/template/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
)

add_subdirectory(third_party/FreeRTOS-Kernel)

file(GLOB EXERCISE_SOURCES CONFIGURE_DEPENDS
    "${EXERCISE_DIR}/app/*.c"
)

if(NOT EXERCISE_SOURCES)
    message(FATAL_ERROR "No C sources found in ${EXERCISE_DIR}/app")
endif()

target_sources(${PROJECT_NAME} PRIVATE ${EXERCISE_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${EXERCISE_DIR}/app
    ${CMAKE_SOURCE_DIR}/week1/ex1/app
    ${CMAKE_SOURCE_DIR}/template/Core/Inc
)

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

target_link_libraries(${PROJECT_NAME} PRIVATE
    stm32cubemx
    freertos_kernel
    freertos_config
)
