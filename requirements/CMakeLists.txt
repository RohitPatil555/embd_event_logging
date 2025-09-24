# --------------------------------------------------------------------
# Doorstop integration – executed as part of the normal build
# --------------------------------------------------------------------
# 1) Find the doorstop executable (assumes it’s on PATH or in vcpkg/Conan)
find_program(DOORSTOP_EXECUTABLE doorstop
    REQUIRED
    DOC "Doorstop CLI tool"
)

# 2) Where are the requirement files?
set(REQ_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Output directory for generated HTML docs
set(HTML_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}")

# 3) Publish doorstop requirement doc
add_custom_target(
    req_docs
    COMMAND ${DOORSTOP_EXECUTABLE} publish --html --index all "${HTML_OUTPUT}" 
    WORKING_DIRECTORY "${REQ_DIR}"
    COMMENT "Generating Doorstop HTML documentation..."
    VERBATIM
)

# 4) Make sure the validation runs *before* any build step
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    #add_dependencies(EmbdEvtLog req_docs)
endif()


