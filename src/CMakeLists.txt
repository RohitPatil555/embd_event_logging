# src/CMakeLists.txt
#add_library(embdEventLog STATIC)

#target_include_directories(embdEventLog PUBLIC
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
#    $<INSTALL_INTERFACE:include>
#)

# Link with sanitizers if needed (they are added globally already)
if(SANITIZERS)
    #target_link_options(embdEventLog PRIVATE ${SANITIZERS})
endif()

# Export the library for other projects
#export(TARGETS embdEventLog FILE EmbdEventLogConfig.cmake)

