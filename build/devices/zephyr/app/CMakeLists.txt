cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(moddable_xs)

if(NOT DEFINED ENV{MCGEN_DIR})
    message(FATAL_ERROR "MCGEN_DIR environment variable not set")
endif()

include($ENV{MCGEN_DIR}/xs_sources.cmake)

set(MC_MODULE_SOURCES "")
if(DEFINED ENV{MCGEN_DIR})
    message(STATUS "MCGEN_DIR=$ENV{MCGEN_DIR}")
    set(MC_MAKEFILE "$ENV{MCGEN_DIR}/makefile")
    if(EXISTS "${MC_MAKEFILE}")
        set(_mc_module_script "import os\nimport re\nmk = r'${MC_MAKEFILE}'\npat = re.compile(r':\\s+(/[^ \\t]+\\.c)\\b')\nsources = []\nseen = set()\nwith open(mk, 'r', encoding='utf-8') as handle:\n    for line in handle:\n        match = pat.search(line)\n        if not match:\n            continue\n        candidate = match.group(1)\n        if candidate in seen:\n            continue\n        seen.add(candidate)\n        sources.append(candidate)\nprint('\\n'.join(sources))")
        execute_process(
            COMMAND python3 -c "${_mc_module_script}"
            OUTPUT_VARIABLE MC_MODULE_SOURCES
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE MC_MODULE_SOURCES_RESULT
        )
        message(STATUS "MC module source discovery result=${MC_MODULE_SOURCES_RESULT} raw='${MC_MODULE_SOURCES}'")
        if(NOT MC_MODULE_SOURCES_RESULT EQUAL 0)
            message(WARNING "Failed to discover MC module sources: ${MC_MODULE_SOURCES_RESULT}")
        elseif(MC_MODULE_SOURCES)
            string(REPLACE "\n" ";" MC_MODULE_SOURCES "${MC_MODULE_SOURCES}")
            message(STATUS "Zephyr MC module sources: ${MC_MODULE_SOURCES}")
        endif()
    endif()
endif()

target_sources(app PRIVATE
    src/main.c
    src/xsmain.c
    src/debugger.c
    ${MODDABLE_MC_XS}
    ${MODDABLE_MC_RESOURCES}
    ${XS_PLATFORM_SOURCES}
    ${XS_RUNTIME_SOURCES}
    ${MC_SOURCES}
    ${MC_MODULE_SOURCES}
)

target_include_directories(app PRIVATE ${MC_INCLUDE_DIRS})
target_compile_definitions(app PRIVATE _DEFAULT_SOURCE=1 ${MC_COMPILE_DEFINITIONS})

if(TARGET native_simulator)
    set(libapp_path "${APPLICATION_BINARY_DIR}/app/libapp.a")
    set_property(TARGET native_simulator APPEND PROPERTY RUNNER_LINK_LIBRARIES "${libapp_path}")
endif()
