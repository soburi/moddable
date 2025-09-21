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
    src/debugger.c
    ${XS_PLATFORM_SOURCES}
    ${XS_RUNTIME_SOURCES}
    ${MC_SOURCES}
)

#set(_mc_all_include_dirs ${MC_INCLUDE_DIRS})
#foreach(_mc_source IN LISTS MC_SOURCES)
#    get_filename_component(_mc_dir "${_mc_source}" DIRECTORY)
#    if(_mc_dir)
#        list(APPEND _mc_all_include_dirs "${_mc_dir}")
#    endif()
#endforeach()
#list(REMOVE_DUPLICATES _mc_all_include_dirs)

foreach(mc_inc IN LISTS MC_INCLUDE_DIRS)
	message(WARNING ${mc_inc})
endforeach()
#message(FATAL_ERROR ${MC_INCLUDE_DIRS})

target_include_directories(app PRIVATE ${MC_INCLUDE_DIRS})
target_compile_definitions(app PRIVATE _DEFAULT_SOURCE=1 ${MC_COMPILE_DEFINITIONS})
