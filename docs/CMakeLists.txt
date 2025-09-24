find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_custom_target(docs
    COMMAND ${Python3_EXECUTABLE} -m mkdocs build --clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building MkDocs documentation"
)

