# SPDX-License-Identifier: MIT | Author: Rohit Patil
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(INPUT_YAML "${EVENT_DESCRIPTION_FILE}")

if (EXISTS "${EVENT_GENERATED_OUT_DIR}")
    set(OUTPUT_DIR "${EVENT_GENERATED_OUT_DIR}")
else ()
    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/exported")
endif ()

set(EVENT_TYPE_HEADER "${OUTPUT_DIR}/event_types.hpp")
set(EVENT_BABELTRACE_CONFIG "${OUTPUT_DIR}/metadata")

# Custom command to run the Python script
add_custom_command(
    OUTPUT "${EVENT_TYPE_HEADER}" "${EVENT_BABELTRACE_CONFIG}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR}"
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_LIST_DIR}/../tools/event_generate.py"
            "${INPUT_YAML}" "${OUTPUT_DIR}"
    DEPENDS "${INPUT_YAML}" "${CMAKE_CURRENT_LIST_DIR}/../tools/event_generate.py"
    COMMENT "Running Python tool to generate header and config"
    VERBATIM
)

# Add a target that depends on generated files
add_custom_target(generate_config
    DEPENDS "${EVENT_TYPE_HEADER}" "${EVENT_BABELTRACE_CONFIG}"
)
