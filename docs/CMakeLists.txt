find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_program(DOORSTOP_EXECUTABLE doorstop
    REQUIRED
    DOC "Doorstop CLI tool"
)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/docs/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/docs")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/mkdocs.yml" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

set(SITE_OUTPUT "${CMAKE_BINARY_DIR}/docs_output")

add_custom_target(
    req_docs
    COMMAND ${DOORSTOP_EXECUTABLE} publish --html --index all "${CMAKE_CURRENT_BINARY_DIR}/docs/static-reqs" 
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/reqs"
    COMMENT "Generating Doorstop HTML documentation..."
    VERBATIM
)

add_custom_target(docs
    COMMAND ${Python3_EXECUTABLE} -m mkdocs build --clean --site-dir ${SITE_OUTPUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building MkDocs documentation"
)

add_dependencies(docs req_docs)
