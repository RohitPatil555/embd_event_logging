# SPDX-License-Identifier: MIT | Author: Rohit Patil
find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_program(DOORSTOP_EXECUTABLE doorstop
    REQUIRED
    DOC "Doorstop CLI tool"
)
# Find Graphviz 'dot' executable
find_program(DOT_EXECUTABLE dot)

if(NOT DOT_EXECUTABLE)
    message(FATAL_ERROR "Graphviz 'dot' not found. Please install Graphviz.")
endif()

set(IMAGES_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs/img)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/docs/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/docs")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/mkdocs.yml" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(MAKE_DIRECTORY ${IMAGES_BINARY_DIR})
#file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/images/*.png" DESTINATION "${IMAGES_BINARY_DIR}")

set(SITE_OUTPUT "${CMAKE_BINARY_DIR}/docs_output")

add_custom_target(
    req_docs
    COMMAND ${DOORSTOP_EXECUTABLE} publish --html --index all "${CMAKE_CURRENT_BINARY_DIR}/docs/static-reqs"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/reqs"
    COMMENT "Generating Doorstop HTML documentation..."
    VERBATIM
)

# Search recursively for all .dot files under the project source
file(GLOB_RECURSE DOT_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/images
    ${CMAKE_CURRENT_SOURCE_DIR}/images/*.dot)


set(ALL_DIAGRAM_TARGETS "")


foreach(dotfile ${DOT_FILES})
    get_filename_component(name ${dotfile} NAME_WE)
    set(out_img ${IMAGES_BINARY_DIR}/${name}.png)

    add_custom_command(
        OUTPUT ${out_img}
        COMMAND ${DOT_EXECUTABLE} -Tpng ${CMAKE_CURRENT_SOURCE_DIR}/images/${dotfile} -o ${out_img}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/images/${dotfile}
        COMMENT "Generating ${name}.png from ${dotfile}"
        VERBATIM
    )

    add_custom_target(${name}_diagram DEPENDS ${out_img})
    list(APPEND ALL_DIAGRAM_TARGETS ${name}_diagram)
endforeach()

add_custom_target(docs
    COMMAND ${Python3_EXECUTABLE} -m mkdocs build --clean --site-dir ${SITE_OUTPUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building MkDocs documentation"
)

add_dependencies(docs req_docs ${ALL_DIAGRAM_TARGETS})
