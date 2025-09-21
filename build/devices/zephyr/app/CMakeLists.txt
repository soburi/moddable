cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(moddable_xs)

if(NOT DEFINED ENV{MCGEN_DIR})
    message(FATAL_ERROR "MCGEN_DIR environment variable not set")
endif()

include($ENV{MCGEN_DIR}/xs_sources.cmake)

target_sources(app PRIVATE
    src/main.c
    src/xsmain.c
    ${XS_PLATFORM_SOURCES}
    ${XS_RUNTIME_SOURCES}
    ${MC_SOURCES}
)

target_include_directories(app PRIVATE ${MC_INCLUDE_DIRS})
target_compile_definitions(app PRIVATE ${MC_COMPILE_DEFINITIONS} XSPLATFORM="zephyr/xsPlatform.h")
